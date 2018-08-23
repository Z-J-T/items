#include<stdio.h>
#include<unistd.h>
#include<assert.h>
#include<pthread.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<stdlib.h>
#include<dirent.h>

#define SER_IP "127.0.0.1"
#define SER_PORT 6000
#define PATH "/home/lol/mydir/FTP/clifile"

//函数声明
int mystrcmp(char buff[]);
void get(int sockfd);
void upload(int sockfd, int count);
char* search(char* name, int count);
int set();
void func();
void samename(int sockfd);

//用来存储外部文件读入的(md5，name)
typedef struct MD5
{
	char passwd[128];
	char name[10];
}md5;

md5 p[1000] = { 0 };

//屏蔽shell其他命令，只实现对文件的管理功能
char* Shell[] = { "rm","cp","mv","touch" };
int shell_count = sizeof(Shell) / sizeof(Shell[0]);
int mystrcmp(char buff[])
{
    if(strncmp(buff,"ls",2) == 0)
    {
        strcpy(buff,"ls -l --color");
        return 1;    
    }
	int i = 0;
	for (; i < shell_count; ++i)
	{
		if (strncmp(buff, Shell[i], strlen(Shell[i])) == 0)
		{
			return 1;
		}
	}
    return 0;
}

//进程替换为脚本文件  目的：生成当前路径下的（MD5，name）文件
void func()
{
	pid_t pid = fork();
	assert(pid != -1);

	//子进程
	if (pid == 0)
	{
		char* argv[1] = { 0 };
		execv("/home/lol/mydir/FTP/clifile/sup.sh", argv);
		perror("execv error!");
		exit(0);
	}
}

//把外部存储（MD5,name）的文件读到全局的数组中去
int set()
{
    int fd=open("my_md5.c",O_RDONLY);
    if(fd == -1)
    {
        printf("open error");
        return;    
    }
    int n=0;
    char buff[1024] = {0};
    while((n=read(fd,buff,1023))>0){}
    char *ptr = NULL;
    char *s =strtok_r(buff,"\n",&ptr);
    strcpy(p[0].passwd,s);
    int i=1;
    while((s = strtok_r(NULL,"\n",&ptr))!=NULL)
    {
        strcpy(p[i++].passwd,s);    
    }
    int j=0;
    for(;j<i;++j)
    {
        char*q =NULL;
        char*s =strtok_r(p[j].passwd," ",&q);
        strcpy(p[j].name,q+1);
        strcpy(p[j].passwd,s); 
    }
    return i;
}

//输入name在数组中查找MD5值
char* search(char* name,int count)
{
    int i=0;
    for(;i<count;++i)
    {
        if(strcmp(name,p[i].name) == 0)
        {
            return p[i].passwd;
        }
    }
    return NULL;
}

//下载
void get(int sockfd)
{
	char buff[128] = { 0 };
	printf("可下载文件目录:\n");
	while (1)
	{
		memset(buff, 0, strlen(buff));
		recv(sockfd, buff, 127, 0);
		if (strncmp(buff, "dir end!", 8) == 0)
		{
			break;
		}
		send(sockfd, "ok", 2, 0);
		printf("%s", buff);
	}

	printf("输入你要下载的文件名：\n");
	memset(buff, 0, strlen(buff));
	fgets(buff, 128, stdin);
	buff[strlen(buff) - 1] = 0;
	send(sockfd, buff, strlen(buff), 0);

	char status[32] = { 0 };
	if (recv(sockfd, status, 31, 0) <= 0)
	{
		return;
	}

	if (strncmp(status, "ok", 2) != 0)
	{
		printf("not find file\n");
		return;
	}

	int size = 0;
	sscanf(status + 3, "%d", &size);
	int fd = open(buff, O_WRONLY | O_CREAT, 0600);
	if (fd == -1)
	{
		send(sockfd, "err", 3, 0);
		return;
	}
	send(sockfd, "ok", 2, 0);

	int curr_size = 0;
	char recvbuff[256] = { 0 };
	int num = 0;

	if (size>0)
	{
		while ((num = recv(sockfd, recvbuff, 256, 0))>0)
		{
			curr_size += num;
			write(fd, recvbuff, num);
			float f = curr_size*100.0 / size;
			printf("down:%2.f%%\r", f);
			fflush(stdout);
			if (curr_size == size)

			{
				printf("\n下载完成!\n");

				return;
			}
		}
	}
	else
	{
		recv(sockfd, recvbuff, 255, 0);
		if (strncmp(recvbuff, "end", 3) == 0)
		{
			printf("下载完成!\n");
		}
	}
}

//上传
void upload(int sockfd, int count)
{
	char *dir = PATH;
	DIR *dp = opendir(dir);
	struct dirent *entry;
	if (dp == NULL)
	{
		fprintf(stderr, "cannot open dirctory:%s\n", dir);
		return;
	}
	printf("本地可上传文件目录:\n");
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(".", entry->d_name) == 0 ||
			strcmp("..", entry->d_name) == 0)
		{
			continue;
		}
		else
		{
			printf("%s\n", entry->d_name);
		}
	}

	printf("输入要上传的文件:\n");
	char buff[128] = { 0 };
	fgets(buff, 127, stdin);
	buff[strlen(buff) - 1] = 0;

	int fd = open(buff, O_RDONLY);
	if (fd == -1)
	{
		send(sockfd, "err", 3, 0);
		printf("no such a file!\n");
		return;
	}
	//发送文件名
	send(sockfd, buff, strlen(buff), 0);
	char recv1[32] = { 0 };
	if (recv(sockfd, recv1, 32, 0)<0)
	{
		return;
	}

	//发送md5
	char *md5 = search(buff, count);

	if (md5 == NULL)
	{
		return;
	}

	send(sockfd, md5, strlen(md5), 0);

	char recv2[32] = { 0 };
	if (recv(sockfd, recv2, 32, 0)<0)
	{
		return;
	}
	//在服务器端找到相同md7值
	if (strcmp(recv2, "find") == 0)
	{
		send(sockfd, "ok", 2, 0);
		char recv_buff[128] = { 0 };


		if (recv(sockfd, recv_buff, 127, 0) < 0)
		{
			return;
		}
		if (strncmp(recv_buff, "exit", 4) == 0)
		{
			printf("服务器端存在相同md5值且文件名相同\n");
			return;
		}
		if (strncmp(recv_buff, "same", 4) == 0)
		{
			samename(sockfd);
		}
		printf("md5值相同， 文件名不同， 硬链接成功\n");
		return;
	}

	//////////////////////////////////////////////
	//发送文件大小
	int size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	sprintf(buff, "%d", size);
	send(sockfd, buff, strlen(buff), 0);

	///////////////////////////////////////
	char status[32] = { 0 };
	if (recv(sockfd, status, 31, 0) <= 0)
	{
		close(fd);
		return;
	}
	if (strncmp(status, "bok",3) == 0)
	{
		send(sockfd, "ok", 2, 0);
		char s_buff[32] = { 0 };
		if (recv(sockfd, s_buff, 32, 0) <= 0)
		{
			return;
		}
		int bsize = 0;
		sscanf(s_buff, "%d", &bsize);
		char sendbuff[256] = { 0 };
		lseek(fd, bsize, SEEK_CUR);
		int n = 0;
		int num = bsize;
		while ((n = read(fd, sendbuff, 256))>0)
		{
			num += n;
			send(sockfd, sendbuff, n, 0);
			float f = num*100.0 / size;
			printf("upload:%2.f%%\r", f);
			fflush(stdout);
		}
		close(fd);
		printf("\n发送完成！\n");
		return;
	}

	if (strcmp(status, "same") == 0)
	{
		samename(sockfd);
        memset(status,0,sizeof(status));
        if(recv(sockfd,status,127,0) < 0)
        {
                return;    
        }
	}

	//////////////////////////////////////////////
	
	if (strcmp(status, "ok") != 0)
	{
		printf("default:ser cannot open file!\n");
		close(fd);
		return;
	}
	////////////////////////////////////////
	
	char sendbuff[256] = { 0 };
	int n = 0;
	int num = 0;
	while ((n = read(fd, sendbuff, 256))>0)
	{
		num += n;
		send(sockfd, sendbuff, n, 0);
		float f = num*100.0 / size;
		printf("upload:%2.f%%\r", f);
		fflush(stdout);
	}
	close(fd);
	printf("\n发送完成！\n");
	return;
}

void samename(int sockfd)
{
	while (1)
	{
		printf("有同名文件，请输入新文件名：\n");
		char send_buff[128] = { 0 };
		fgets(send_buff, 127, stdin);
		send_buff[strlen(send_buff) - 1] = '\0';
		send(sockfd, send_buff, strlen(send_buff), 0);
		char recv_buff[128] = { 0 };
		if (recv(sockfd, recv_buff, 127, 0) < 0)
		{
			return;
		}
		if (strcmp(recv_buff, "ok") == 0)
		{
		    return;
		}
	}
}

//主函数
int main()
{
	func();
    int count = set();
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    assert(sockfd != -1);

    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family =AF_INET;
    saddr.sin_port = htons(SER_PORT);
    saddr.sin_addr.s_addr = inet_addr(SER_IP);
    
    int res = connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    assert(res!=-1);

    while(1)
    {
        char buff[128] ={0};
    printf("*******************\n请使用命令(ls rm cp mv touch)\n或者\n功能：\n1.get(下载)\n2.upload(上传)\n3.end(退出)\n*******************\n");
        printf("Connect success>> ");
        fflush(stdout);

        fgets(buff,128,stdin);
        
        buff[strlen(buff)-1]=0;
        
        //退出
        if(strcmp(buff,"end") == 0)
        {
            break;    
        }
        //下载
        else if(strncmp(buff,"get",3) == 0)
        {
            send(sockfd,buff,strlen(buff),0);
            get(sockfd);
			func();
			count = set();
            continue;
        }
        //上传
        else if(strncmp(buff,"upload",6) == 0)
        {
            send(sockfd,buff,strlen(buff),0);
            upload(sockfd,count);
            continue;
        }
        //命令
        else if(mystrcmp(buff) == 1) 
        {
            send(sockfd,buff,strlen(buff),0);
            char recvbuff[1024] ={0};
            recv(sockfd,recvbuff,1023,0);
            printf("%s\n",recvbuff+3);
        }
        else
        {
            printf("输入错误！\n");    
        }
    }
    close(sockfd);
}
