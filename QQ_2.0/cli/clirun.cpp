#include"cli.h"

//请求类型 
enum 
{
    TYPE_LOGIN = 1,
    TYPE_REGISTER,
    TYPE_EXIT,
    TYPE_GETLIST,
    TYPE_TALKONE,
    TYPE_TALKGROUP,
};

typedef void (*PFUNC)(int);
typedef struct
{
    int choice;
    PFUNC pfunc;
}ElemType;

static pthread_t id;

//菜单选项
const int LOGIN = 1;
const int REGISTER = 2;
const int EXIT = 3;

ElemType MenuTable[] = 
{
    {LOGIN,&Login},
    {REGISTER,&Register},
    {EXIT,&Exit},
};

int MenuTablesize = sizeof(MenuTable)/sizeof(MenuTable[0]);

volatile bool flag = true;

void run(int fd)
{	
    int choice;
    flag = true;
	while(flag)
	{	
        cout<<"-----------------------------------"<<endl;
		cout<<"QQ"<<endl;
        cout<<"------------------"<<endl;
        cout<<"1.Login"<<endl;
        cout<<"2.Register"<<endl;
        cout<<"3.Exit"<<endl;        
        cout<<"请输入:";
        
        char buff[128]="";
        //fgets(buff,127,stdin);
        cin>>buff;
        if(isdigit(buff[0]) && (choice = atoi(buff)) <= 3 && choice >0)
        {       
            for(int i = 0;i<MenuTablesize ;++i)
		    {
                if(choice == MenuTable[i].choice)
                {
    		    	(*MenuTable[i].pfunc)(fd);
    	        }
	        }
        }
        else
        {
            cout<<"输入错误！"<<endl; 
        }
    }
}

void Login(int fd)
{
    cout<<"-----------------------------------"<<endl;
    cout<<"Login"<<endl;
    cout<<"------------------"<<endl;
    char name[20] ={0};
	cout<<"please cin name:"<<endl;
	cin>>name;

	char pw[20] ={0};
	cout<<"please cin password"<<endl;
	cin>>pw;

    //json 把类型 name pw 打包
	Json::Value val;
    val["type"] = TYPE_LOGIN;
    val["name"] = name;
    val["pw"] = pw;
    
	//将其发送给服务器
    if( -1 == send(fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
    {
    	cerr<<"send reason fail;errno:"<<errno<<endl;
    	return;
    }

	//接受服务器的回应
	char buff[128] = "";

    if(recv(fd,buff,127,0)<0)
    {
        cout<<"------------------"<<endl;
        cout<<"反馈异常！"<<endl;
        return ;
    }
	if(strcmp(buff,"ok") == 0)
 	{
        //两个线程 一个接收 一个发送(主线程)
        pthread_create(&id,NULL,thread_recv,(void*) fd);
        cout<<"登录成功！"<<endl;
		login_success(fd);
	}	
	else
    {
        //失败->
        cout<<"登录失败"<<endl;
    }
}

//登陆成功
const int GET_LIST = 1;
const int TALK_ONE = 2;
const int TALK_GROUP = 3;
const int L_EXIT = 4;


ElemType LOGINTABLE[] = 
{
    {GET_LIST,&Get_List},
    {TALK_ONE,&Talk_One},
	{TALK_GROUP,&Talk_Group},
    {L_EXIT,&L_Exit}
};

int LOGINTABLESIZE = sizeof(LOGINTABLE)/sizeof(LOGINTABLE[0]);

volatile bool flag1 = true;

void login_success(int fd)
{ 
    int choice;
    flag1 = true;
    while(flag1)
    {
        cout<<"-----------------------------------"<<endl;
        cout<<"功能列表："<<endl;
        cout<<"------------------"<<endl;

	    cout<<"1.get list"<<endl;
	    cout<<"2.talk one"<<endl;
	    cout<<"3.talk group"<<endl;
        cout<<"4.exit"<<endl;
        //cout<<"请输入："; 
       
        char buff[128]="";
        //fgets(buff,127,stdin);
        cin>>buff;
        if(isdigit(buff[0]) && (choice = atoi(buff)) <= 4 && choice >0)
        {
    	    for(int i = 0;i<LOGINTABLESIZE ;++i)
    	    {
                if(choice == LOGINTABLE[i].choice)
                {
      		        (*LOGINTABLE[i].pfunc)(fd);
                }
    	    }
        }
        else
        {
            cout<<"输入错误！"<<endl;    
        }
    }
}

void Register(int fd)
{
    char name[20] = {0};
	char pw[20] = {0};
    while(1)
    {
        cout<<"-----------------------------------"<<endl;
	    cout<<"Register"<<endl;
        cout<<"------------------"<<endl;
        cout<<"please cin name:"<<endl;
	    cin>>name;
            
        char ps[20] = {0};
        cout<<"please cin password:"<<endl;
        cin>>ps;

        cout<<"please cin password again:"<<endl;
	    cin>>pw;
    
        if(strcmp(ps,pw) == 0)
        {
            break;
        }
        cout<<"两次密码不同"<<endl;
        memset(ps,0,20);
        memset(pw,0,20);
    }

    //json 把类型 name pw 打包
	Json::Value val;
    val["type"] =TYPE_REGISTER;
    val["name"] = name;
    val["pw"] = pw;
    
	//将其发送给服务器
    if( -1 == send(fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0) )
    {
    	cerr<<"send reason fail;errno:"<<errno<<endl;
    	return;
    }

	//接受服务器的回应
	char buff[128] = "";
    if(0 >recv(fd,buff,127,0))
    {
        cout<<"------------------"<<endl;
        cout<<"反馈异常！"<<endl;
        return;
    }	    
	//成功->
	if(strcmp(buff,"ok") == 0)
 	{
        cout<<"注册成功"<<endl;
	}	
    //失败->
    else
    {
        cout<<"failed!"<<endl;
    }
    return;
}

void Exit(int fd)
{
    flag = false;
}

void Get_List(int fd)
{
	Json::Value val;
    val["type"] =TYPE_GETLIST;
    
    if(-1 == send(fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
    {
    	cerr<<"send reason fail;errno:"<<errno<<endl;
    	return;   
    }
}

void *thread_recv(void* arg)
{
    int fd = (int)arg;
    char buff[128]="";
    while(1)
    {
        if(recv(fd,buff,127,0) > 0)
        {
            Json::Value val;
            Json::Reader read;
            if(read.parse(buff,val) == -1) 
            {
                cout<<"read error"<<endl;
                break;
            }
            int type = atoi((char*)val["type"].toStyledString().c_str());

            if(type == 0)
            {
                cout<<"-------------------"<<endl;
                cout<<"在线列表:"<<endl;
            }
            
            cout<<val["data"].asString()<<endl;
            cout<<"-------------------"<<endl;
        }
        else
        {
            break;
        }
    }
}

void Talk_One(int fd)
{
    cout<<"-------------------"<<endl;
    cout<<"请输入要聊天对象:"<<endl;
    
    char name[32] = "";
    cin>>name;
    
    cout<<"-------------------"<<endl;
    cout<<"开始聊天：end 结束"<<endl;
    cout<<"-------------------"<<endl;

    Json::Value val;
    val["type"] =TYPE_TALKONE;
    char data[128]="";
    cout<<endl;
    

    val["name"] = name;
    while(1)
    {

        cin>>data;
        
        if(strncmp(data,"end",3) == 0)
        {
            break;    
        }
        
        val["data"] = data;

        if(-1 == send(fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
        {
    	    cerr<<"send reason fail:errno:"<<errno<<endl;
    	    return;   
        }
    }
}

void Talk_Group(int fd)
{
    cout<<"-------------------"<<endl;
    cout<<"群发：end 结束"<<endl;
    cout<<"-------------------"<<endl;

    while(1)
    {
        Json::Value val;
        val["type"] =TYPE_TALKGROUP;
        char data[128]="";
        cout<<endl;
        cin>>data;
        
        if(strncmp(data,"end",3) == 0)
        {
            break;    
        }
        

        val["data"] = data;

        if(-1 == send(fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
        {
    	    cerr<<"send reason fail:errno:"<<errno<<endl;
    	    return;   
        }
    }
}

void L_Exit(int fd)
{
    flag1 = false;
    
	Json::Value val;
    val["type"] =TYPE_EXIT;
        
	//将其发送给服务器
    if( -1 == send(fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0) )
    {
    	cerr<<"send reason fail;errno:"<<errno<<endl;
    	return;
    }
    
    pthread_cancel(id);
    cout<<"---------------"<<endl;
    cout<<"byebye!"<<endl;	    
    cout<<"---------------"<<endl;
}
