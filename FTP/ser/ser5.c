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

#define SER_PORT 6000
#define SER_IP "127.0.0.1"
#define PATH "/home/lol/mydir/FTP/serfile"

//函数声明
int search_name(char* name,int count);
int socket_create();
void* thread_fun(void *arg);
void get(int c);
void upload(int c);
char* search(char* md5,int count);
int search_name(char* name, int count);
int set();
void func();
void samename(int c,char buff[]);

//用来存储外部文件读入的(md5，name)
typedef struct MD5
{
	char* passwd;
	char* name;
}md5;

md5 p[1000] = { 0 };
static int count = 0;

//用来记录断点信息（MD5，已经上传的size）
typedef struct BrokenFile
{
	char passwd[128];
	int size;
}brokenfile;

brokenfile broken[10] = { 0 };
static int B_count = 0;

//创建套接字
int socket_create()
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        return -1;    
    }

    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family =AF_INET;
    saddr.sin_port = htons(SER_PORT);
    saddr.sin_addr.s_addr = inet_addr(SER_IP);
    int res = bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    assert(res!=-1);

    listen(sockfd,5);
    return sockfd;
}

//线程函数
void* thread_fun(void* arg)
{
    int c =(int)arg;
	//
    while(1)
    {
        char buff[128] ={0};
        int n = recv(c,buff,127,0);
        if(n <= 0)
        {
            break;
        }
        
        char *myargv[10]={};
        char *ptr =NULL;
        char *s = strtok_r(buff," ",&ptr);
        if(s == NULL)
        {
            continue;    
        }
        else
        {
            myargv[0] = s;   
        }
        int i=1;
        while((s = strtok_r(NULL," ",&ptr)) != NULL )
        {
            myargv[i++] = s;    
        }
        
        if(strcmp(myargv[0],"get") == 0)
        {
            get(c);
            continue;
        }
        if(strcmp(myargv[0],"upload") == 0)
        {
            upload(c);
            func();
            count = set();
            continue;    
        }
        else
        {       
            int fd[2];
            assert(pipe(fd) != -1);

            pid_t pid =fork();
            assert(pid != -1);
            
            //子进程
            if(pid == 0)
            {
                close(fd[0]);
                dup2(fd[1],1);
                dup2(fd[1],2);

                char bu[128] ={0};
                strcpy(bu,"/bin/");
                strcat(bu,myargv[0]);
                execv(bu,myargv);
                perror("execv error!");
                exit(0);        
            }
            else 
            {
                close(fd[1]);
                wait(NULL);
                char sendbuff[1024] ={"ok#"};
                read(fd[0],sendbuff+3,1021);
                send(c,sendbuff,strlen(sendbuff),0);
                close(fd[0]);
                func();
                count = set();
            }
        }
    }
    close(c);
    printf("one client over!\n");

}

//下载
void get(int c)
{
    char buff[128] ={0};
    char *dir = PATH;
    DIR *dp = opendir(dir);
    struct dirent *entry;
    if( dp == NULL)
    {
        fprintf(stderr,"cannot open dirctory:%s\n",dir);
        return;
    }
    while((entry = readdir(dp))!=NULL)
    {
        if(strcmp(".",entry->d_name) == 0 || 
            strcmp("..",entry->d_name) == 0)    
        {
            continue;    
        }
        else
        {
            memset(buff,0,strlen(buff));
            strcpy(buff,entry->d_name);
            buff[strlen(buff)] = '\n';
            send(c,buff,strlen(buff),0);
            char recv_buff[32] ={0};  
            if(recv(c,recv_buff,31,0)<0)
            {
                return;    
            }
        }
    }
    memset(buff,0,strlen(buff));
    send(c,"dir end!",8,0);

    memset(buff,0,strlen(buff));
    if(recv(c,buff,127,0) <= 0)
    {
        return;
    }
    int fd = open(buff,O_RDONLY);

    if(fd == -1)
    {
        send(c,"err",3,0);
        return;    
    }

    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    char st_buff[32] = {0};
    sprintf(st_buff,"ok#%d",size);

    send(c,st_buff,strlen(st_buff),0);
    char status[32] = {0};
    if(recv(c,status,31,0) <= 0)
    {
        close(fd);
        return;
    }

    if(strcmp(status,"ok") != 0)
    {
        close(fd);
        return;   
    }
    char sendbuff[256] ={0};
    int n = 0;
    if(size == 0)
    {
        send(c,"end",3,0);    
    }
    else
    {
        sendfile(c,fd,NULL,size);
    }
    close(fd);
}

//上传
void upload(int c)
{
    func();
    count=set();
    char buff[128] ={0};
    //接收文件名
    if(recv(c,buff,127,0) <=0)
    {
        return;    
    }
    if(strncmp(buff,"err",3) == 0)
    {
        return;    
    }
    
    send(c,"ok",2,0);
    //接收md5
    char mdbuff[128] ={0};
    if(recv(c,mdbuff,128,0)<0)
    {
        return;    
    }
    
    char* name=NULL;

    //搜索库中文件md5值对应的name 
    name=search(mdbuff,count);
    

    //没有相同的md5值   name为空
    if(name == NULL)
    {
        send(c,"error",4,0);    
    }
    //有相同的MD5值 返回空中MD5对应的name
    else
    {
        send(c,"find",4,0);
        //在服务器找到相同md5值的文件
        
        char b[32] = {0};
        if(recv(c,b,31,0)<0)
        {
            return;    
        }

		//md5值相同 文件名也相同
        if(strcmp(name,buff) == 0)
        {
            send(c,"exit",4,0);
            return;    
        }
		else	//MD5值	文件名不同 硬链接	
		{
			//库中搜索有无buff  同名的文件
			int n = search_name(buff, count);
			if (n == 1)
			{
				//md5值相同 文件名不同 和库中有同名的文件
				send(c, "same", 4, 0);
				samename(c,buff);
			}
			else
			{	//md5值相同 文件名不同 且和库中没有同名的文j
				send(c, "ok", 2, 0);
			}
			
			pid_t pid = fork();
			assert(pid != 1);

			if (pid == 1)
			{
				char *argv[10] = { 0 };
				argv[0] = "ln";
				argv[1] = name;
				argv[2] = buff;
				execvp(argv[0], argv);
				perror("execv error");
				exit(0);
			}
			wait(NULL);
			return;
		}
    }
   

    //////////////////////////////////////////////////////////
    //没有找到相同md5的文件
    
    //接收文件大小
    char s_buff[32] ={0};
    if(recv(c,s_buff,32,0)<=0)
    {
        return;    
    }
        
    int size = 0;
    sscanf(s_buff,"%d",&size);
    
	/////////////////////////////////////////////////////////////
	//在broken中找相同的md5
	int oldsize = search_broken(mdbuff);
    printf("!!!!!!!\n");
	if (oldsize != 0)
	{

        printf("111111111111\n");
		int fd = open(buff, O_WRONLY);
		if (fd == -1)
		{
			send(c, "berr", 3, 0);
			return;
		}
		send(c, "bok", 3, 0);

		char re_bu[32] = { 0 };
		if (recv(c, re_bu, 31, 0) < 0)
		{
			return;
		}
		memset(re_bu, 0, sizeof(re_bu));
		sprintf(re_bu, "%d", oldsize);
		send(c, re_bu, strlen(re_bu), 0);

		lseek(fd, 0, SEEK_END);
		char recvbuff[256] = { 0 };
        int num =0;
		int i=search_broken_index(mdbuff);
		while ((num = recv(c, recvbuff, 256, 0))>0)
		{
			oldsize += num;
			write(fd, recvbuff, num);
			if (oldsize == size)
			{
				close(fd);
                strcpy(broken[i].passwd,"\0");    
               // B_count--;
				return;
			}
		}
        close(fd);
		if (oldsize != size)
		{
			broken[i].size = oldsize;
		}
		return;
	}


	/////////////////////////////////////
	//如果在库中存在相同的文件名
	int n = search_name(buff, count);
	if (n)
	{
		send(c, "same", 4, 0);
		samename(c,buff);
	}
	/////////////////////////////////////////////////

    int fd = open(buff,O_WRONLY | O_CREAT,0600);
    if(fd == -1)
    {
        send(c,"err",3,0);
        return;      
    }
    send(c,"ok",2,0);
    int cur_size=0;
    int num =0;
    char recvbuff[256] = {0};
    while((num = recv(c,recvbuff,256,0))>0)
    {
        cur_size +=num;
        write(fd,recvbuff,num);
        if(cur_size == size )
        {
            printf("2\n");
            close(fd);
            return;
        }
    }
    close(fd);
	if (cur_size != size)
	{
        printf("1\n");
		strcpy(broken[B_count].passwd,mdbuff);
		broken[B_count++].size = cur_size;
	}
}

void samename(int c,char buff[])
{
	while (1)
	{
		memset(buff, 0, sizeof(buff));
		if (recv(c, buff, 127, 0) < 0)
		{
			return;
		}		
		int n = search_name(buff, count);
		if (!n)
		{
			send(c, "ok", 2, 0);
			return;
		}
		send(c, "err", 3, 0);
	}
}

//输入MD5返回，如果有记录，返回断点大小
int search_broken(char* mdbuff)
{
	int i = 0;
	for (; i < B_count; ++i)
	{
		if (strncmp(mdbuff, broken[i].passwd,strlen(mdbuff)) == 0)
		{
			return broken[i].size;
		}
	}
	return 0;
}

//输入MD5返回存放该数据的下标
int search_broken_index(char mdbuff[])
{
	int i = 0;
	for (; i < B_count; ++i)
	{
		if (strncmp(mdbuff, broken[i].passwd,strlen(mdbuff)) == 0)
		{
			return i;
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
		execv("/home/lol/mydir/FTP/serfile/sup.sh", argv);
		perror("execv error!");
		exit(0);
	}
}

//把外部存储（MD5,name）的文件读到全局的数组中去
int set()
{
    count =0;
    int fd=open("my_md5.c",O_RDONLY);
    if(fd == -1)
    {
        printf("open error");
        return;    
    }
    int n=0;
    char buff[1024] = {0};
    while((n=read(fd,buff,1023))>0)
    {}
    char *ptr = NULL;
    char *s = strtok_r(buff,"\n",&ptr);
    p[0].passwd=s;
    int i = 1;
    while((s = strtok_r(NULL,"\n",&ptr)) != NULL)
    {
        p[i++].passwd=s;    
    }
    int j=0;
    for(;j<i;++j)
    {
        char* q =NULL;
        char* s=strtok_r(p[j].passwd,"  ",&q);
        p[j].name=q+1;
        p[j].passwd=s;
    }
    return i;
}

//通过MD5值搜索name
char* search(char* md5,int count)
{
    int i=0;
    for(;i<count;++i)
    {
        if(strcmp(md5,p[i].passwd) == 0)
        {
            return p[i].name;
        }
    }
    return NULL;
}

//输入name返回是否存在
int search_name(char* name,int count)
{
    int i=0;
    for(;i<count;++i)
    {
        if(strcmp(name,p[i].name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

//主函数
int main()
{
	func();
	count = set();
	int sockfd = socket_create();

	struct sockaddr_in caddr;
	while (1)
	{
		int len = sizeof(caddr);
		int c = accept(sockfd, (struct sockaddr*)&caddr, &len);
		if (c<0)
		{
			continue;
		}
		printf("accept c = %d\n", c);

		pthread_t id;
		pthread_create(&id, NULL, thread_fun, (void*)c);
	}
}
