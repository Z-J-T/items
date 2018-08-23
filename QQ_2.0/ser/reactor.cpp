#include"ser.h"

Ser Ser::_ser;
Contral* Childthread::con=NULL;

//创建套接字
static int socket_create(const char *ip,int port)
{
    cout<<port<<endl;
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        return -1;
    }

    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);

    int res = bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    assert(res != -1);

    return sockfd;
}

//主线程
void * thread_fun(void *arg)
{
    cout<<Ser::Getser()<<endl;
    int s1 = (int) arg;

    cout<<"thread_fun s1="<<s1<<endl;
    
    Ser::Getser()->_childpth[s1] = new Childthread(s1);
}


//构造
Ser::Ser()
{	
    int pthread_num = 3;
    const char* ip="127.0.0.1";
    const int port = 8000;

    cout<<Getser()<<endl;

    //libevent
	_base = event_base_new();
	
	_socketpair.reserve(pthread_num);

    //循环创建子线程
    int i=0;
    
    pthread_t *id =(pthread_t*)malloc(sizeof(pthread_t)*pthread_num);
    
    for(;i<pthread_num;++i)
    {
		//创建双向管道
        if( socketpair(AF_UNIX , SOCK_STREAM , 0 , _socketpair[i]) == -1 )
        { 
            printf("create unnamed socket pair failed:%s\n",strerror(errno)); 
            exit(-1); 
        }
        cout<<"sockpair 0 :"<<_socketpair[i][0]<<endl;    
        cout<<"sockpair 1 :"<<_socketpair[i][1]<<endl;
        _pth_count.insert(make_pair(_socketpair[i][0],0));
    
        //创建子线程
        pthread_create(&id[i],NULL,thread_fun,(void *)_socketpair[i][1]);    
	}

    //创建套接字
	int fd =socket_create(ip,port);
	if(fd == -1)
	{
        cerr<<"fd creat fail:errno"<<errno<<endl;
        return;
	}
	
    //监听
    if(-1 == listen(fd,20))
    {
        cerr<<"listen fail:errno:"<<errno<<endl;
        return;
    }
    
    cout<<"create_s  fd = "<<fd<<endl;

    struct event* listen_event=event_new(_base,fd,EV_READ|EV_PERSIST,listen_cb, _base);
    if(NULL == listen_event)
    {
        cerr<<"event new fail:errno:"<<errno<<endl;
        return;
    }

    event_add(listen_event,NULL); 
    event_base_dispatch(_base);
}

//析构
Ser::~Ser()
{}

//有客户端连接的回调函数
void listen_cb(int fd,short event,void* arg)
{
    cout<<"listen_cb"<<endl;

    cout<<"listen_cb fd "<<fd<<endl;

    struct event_base *_base = (struct event_base*)arg;
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    
    int cli_fd = accept(fd,(struct sockaddr*)&caddr,&len); 
    if(-1 == cli_fd)
    {
        cerr<<"accept fail;errno"<<errno<<endl;
        return;
    }

    cout<<"accept -> cli_fd = "<<cli_fd<<endl;

	//选择负载量最小的线程把新的cli_fd写入
	 
    cout<<endl;
    cout<<"best one"<<endl;
    int best_fd = Ser::Getser()->_socketpair[0][0];
    int best_one = Ser::Getser()->_pth_count[best_fd];
    for(int i=1;i<3;++i)
    {
        if(best_one > Ser::Getser()->_pth_count[Ser::Getser()->_socketpair[i][1]])
        {
            best_fd = Ser::Getser()->_socketpair[i][0];
            best_one =Ser::Getser()->_pth_count[best_fd];    
        }
    }

    cout<<"write"<<endl;
	//往负载量最小的s1上写，把新产生的文件描述符传给子线程
	char buff[128] = "";
	int w;
	sprintf(buff, "%d", cli_fd);
    cout<<"buff = "<<buff<<endl;
    
    cout<<"write in "<<best_fd<<endl;
    
	if ((w = write(best_fd , buff, strlen(buff))) == -1) 
    {
		printf("Write socket error:%s\n", strerror(errno));
		exit(-1);
	}
    
    cout<<"read"<<endl;
    int r;
	//从s0
    char size_buff[32] = "";
	if ((r = read(best_fd , size_buff,31)) == -1) 
    {
	    printf("Read from socket error:%s\n", strerror(errno));
	    exit(-1);
    }
    
    cout<<"read end"<<endl;
    cout<<size_buff<<endl;
    
    // 更新_pth_count
    int count = atoi(size_buff);
    
    cout<<"count = "<<count<<endl;
    
    Ser::Getser()->_pth_count[best_fd] = count;
}

///////////////////////////////////////////////////////////////////////
//子线程


Childthread::Childthread(int s1)
{
    cout<<"childthread(s1) "<<this<<endl;
	cout<<"child - s1 "<<s1<<endl;
    
    Ser::Getser()->_childpth[s1] = this;
    
    //创建libevent
	_base = event_base_new();

	//把s1端监听起来
	struct event* s1_event = event_new(_base, s1, EV_READ | EV_PERSIST, func_cb, _base);
	if (NULL == s1_event)
	{
		cerr << "event new fail:errno:" << errno << endl;
		return;
	}
    cout<<"c - s1 = "<<s1<<endl;
	event_add(s1_event, NULL);
    //cout<<_base->event_count<<endl;;
	event_base_dispatch(_base);
}

Childthread::~Childthread()
{
	//循环释放所有的事件_event_map
	map<int, struct event*>::iterator it = _event_map.begin();

	for (; it != _event_map.end(); ++it)
	{
		event_free(it->second);
	}
	//删除

	event_base_free(_base);

}

typedef struct Need
{
    int fd;
    struct event* eve; 
}Need;

void func_cb(int fd, short event, void* arg)
{
    cout<<"func_cb fd "<<fd<<endl;
    //从管道s1端读到一个文件描述符
    
    struct event_base * _base =(struct event_base*) arg;
	
    char buff[128] = "";
	int r;
	
    if ((r = read(fd, buff, 127)) == -1) 
    {
		printf("Read from socket error:%s\n", strerror(errno));
		exit(-1);
	}

	int cli_fd = atoi(buff);
    
    cout<<"cli_fd "<<cli_fd<<endl;
	//动态创建一个event结构体
	struct event* cli_event = event_new(NULL,-1,0,NULL,NULL);
	//动态创建结构体作为event回调函数
    Need* q = (Need*)malloc(sizeof(Need));
    q->fd = fd;
    q->eve = cli_event;
	event_assign(cli_event, _base,cli_fd,EV_READ|EV_PERSIST,cli_cb,(void*)q);
	
    cout<<"1 "<<cli_fd<<endl;
	if(NULL == cli_event)
	{
	    cerr<<"cli event creat fail:errno:"<<errno<<endl;
	    return ;
	}

    cout<<"2"<<endl;
    
    cout<<"fd = "<<fd<<endl;
    cout<<"cli_fd = "<<cli_fd;

    Ser::Getser()->_childpth[fd]->_event_map[cli_fd] = cli_event;
    
    cout<<"3"<<endl;
    int size =Ser::Getser()->_childpth[fd]->_event_map.size();
	int w;
    char size_buff[32] = "";
	sprintf(size_buff, "%d", size);
    
	if ((w = write(fd , size_buff, strlen(buff))) == -1) 
    {
		printf("Write socket error:%s\n", strerror(errno));
		exit(-1);
    }

	event_add(cli_event,NULL);
}

void cli_cb(int fd,short event,void *arg)
{
    cout<<"cli_cb"<<endl;
    struct Need* q = (struct Need*)arg;
    
    int childfd = q->fd;
    cout<<"childfd"<<childfd<<endl;

    struct event* eve = q->eve;


    char buff[128] = "";
    if(recv(fd,buff,127,0) > 0)
    {
        cout<<buff<<endl;
        Childthread::con = new Contral();
        Childthread::con->process(fd,buff);
    }
    else
    {
        cout<<"cli over!"<<endl;
        event_free(eve);

		//把_event_map中的事件删除
        Ser::Getser()->_childpth[childfd]->_event_map.erase(fd);

        close(fd);       
    }
}

