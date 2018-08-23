#include"ser.h"

Mysql* Childthread::m_mysql = new Mysql("localhost","111111");

//extern pthread_mutex_t mutex;

//MYSQL_RES *res = Childthread::mysql->res;


void Login_view::process(int &fd,char *buff)
{
    cout<<"Login_view"<<endl;

    Json::Value val;
    Json::Reader read;
    if(-1 == read.parse(buff,val))
    {
        cerr<<"read faile errno"<<errno<<endl;
        return;    
    }

    //在user中查找有没有该用户
    if(Childthread::m_mysql->search("user",val["name"].asString().c_str(),val["pw"].asString().c_str()))
    {
        //有该用户
        cout<<"success"<<endl;    
         
        send(fd,"ok",2,0);

        //查找离线列表中有没有自己的数据
        int flag;
        char query[128]="";
        
        //Childthread::m_mysql->mysql_connect();

        sprintf(query,"select * from offline where name = '%s'",val["name"].asString().c_str());
        cout<<query<<endl;
        
        //pthread_mutex_lock(&mutex);
        
        flag = mysql_real_query(Childthread::m_mysql->mysql,query,(unsigned int)strlen(query));
        
        if(flag)
        {
            cout<<"query failed"<<endl;
            return;    
        }
        else
        {

            Childthread::m_mysql->res = mysql_store_result(Childthread::m_mysql->mysql);
            
            Json::Value root;
            char send_buff[128] = "";
                
            Childthread::m_mysql->row = mysql_fetch_row(Childthread::m_mysql->res);
            if(Childthread::m_mysql->row == 0)
            {   
                cout<<"donothing"<<endl;    
            }
            else
            {
                    while(Childthread::m_mysql->row)
                    {
                        int i=0;
                        for(;i<mysql_num_fields(Childthread::m_mysql->res);)
                        {
                            strcat(send_buff,"离线消息 to ");
                            strcat(send_buff,Childthread::m_mysql->row[i++]);
                            strcat(send_buff,Childthread::m_mysql->row[i++]);
                            strcat(send_buff,"\n");    
                        }
                        Childthread::m_mysql->row = mysql_fetch_row(Childthread::m_mysql->res); 
                    }

                    cout<<"data:"<<send_buff;
                    root["data"]=send_buff;
                    send(fd,root.toStyledString().c_str(),strlen(root.toStyledString().c_str()),0);
        
                    //pthread_mutex_unlock(&mutex);
                    
                    //读完离线消息后，删除记录
                    Childthread::m_mysql->mydelete("offline","name",val["name"].asString().c_str());
            }
        }
        char p[20]="";
        sprintf(p,"%d",fd);

        //把用户信息放在online表中
        if(Childthread::m_mysql->insert("online",val["name"].asString().c_str(),p))
        {    
            //send(fd,"ok",2,0);
            return;
        }
    }
    send(fd,"err",3,0);
    cout<<"failed"<<endl;
}

void Register_view::process(int &fd,char *buff)
{
    cout<<fd<<endl;
    Json::Value val;
    Json::Reader read;
    if(-1 == read.parse(buff,val))
    {
        cerr<<"read faile errno"<<errno<<endl;
        return;
    }

    if(Childthread::m_mysql->insert("user",val["name"].asString().c_str(),val["pw"].asString().c_str()))
    {
        cout<<"success"<<endl;
        send(fd,"ok",2,0);
    }
    else
    {
        cout<<"success"<<endl;
        send(fd,"err",3,0);
    }
    cout<<fd<<endl;
}

void Exit_view::process(int &fd,char *buff)
{
    Json::Value val;
    Json::Reader read;
    if(-1 == read.parse(buff,val))
    {
        cerr<<"read faile errno"<<errno<<endl;
        return;    
    }
    
    char p[20]="";
    sprintf(p,"%d",fd);

    Childthread::m_mysql->mydelete("online","fd",p);
}

void Getlist_view::process(int &fd,char *buff)
{
    Json::Value val;
    Json::Reader read;
    if(-1 == read.parse(buff,val))
    {
        cerr<<"read faile errno"<<errno<<endl;
        return;    
    }

    int flag;
    char query[128]="";
    
    Childthread::m_mysql->mysql_connect();
    
    sprintf(query,"select * from online");
    
    //pthread_mutex_lock(&mutex);
    cout<<"query "<<query<<endl;
    flag = mysql_real_query(Childthread::m_mysql->mysql,query,(unsigned int)strlen(query));
    if(flag)
    {
        cout<<"query failed"<<endl;
        return;    
    }
    else
    {
        Childthread::m_mysql->res = mysql_store_result(Childthread::m_mysql->mysql);
        char send_buff[128] = "";
        Json::Value root;
    
        while(Childthread::m_mysql->row = mysql_fetch_row(Childthread::m_mysql->res))
        {
            int i=0;
            for(;i<mysql_num_fields(Childthread::m_mysql->res);)
            {
                strcat(send_buff,"name:");
                strcat(send_buff,Childthread::m_mysql->row[i++]);
                strcat(send_buff," , fd:");
                strcat(send_buff,Childthread::m_mysql->row[i++]);
                strcat(send_buff,"\n");    
            }
        }

        root["data"]=send_buff;

        send(fd,root.toStyledString().c_str(),strlen(root.toStyledString().c_str()),0);
    }
    //pthread_mutex_unlock(&mutex);
}

void Talkone_view::process(int &fd,char *buff)
{
    Json::Value val;
    Json::Reader read;

    cout<<"talk"<<endl;
    if(-1 == read.parse(buff,val))
    {
        cerr<<"read faile errno"<<errno<<endl;
        return;    
    }
    
    //Childthread::mysql->mysql_connect();
    
    char q[256]="";

    sprintf(q,"select * from online where fd = '%d'",fd);
    cout<<"q = "<<q<<endl;

    //pthread_mutex_lock(&mutex);
    char p[128] ="";
    int flag = mysql_real_query(Childthread::m_mysql->mysql,q,(unsigned int)strlen(q));
    if(flag)
    {
        cout<<"query failed"<<endl;    
        return;
    }
    
        cout<<"success"<<endl;
        Childthread::m_mysql->res = mysql_store_result(Childthread::m_mysql->mysql);
        Childthread::m_mysql->row = mysql_fetch_row(Childthread::m_mysql->res);

        if(Childthread::m_mysql->row == 0)
        {
            cout<<"search error"<<endl;
            return;
        }
        
        strcat(p," from ");
        strcat(p, Childthread::m_mysql->row[0]);
        
        cout<<p<<endl;

        char query[128] = "";
        sprintf(query,"select * from online where name='%s'",val["name"].asString().c_str());
        flag = mysql_real_query(Childthread::m_mysql->mysql,query,(unsigned int)strlen(query));
        if(flag)
        {
            cout<<"query failed!"<<endl;    
        }
        else
        {
            Childthread::m_mysql->res = mysql_store_result(Childthread::m_mysql->mysql);
            Childthread::m_mysql->row = mysql_fetch_row(Childthread::m_mysql->res);
        
            char E_buff[256]="";
            strcpy(E_buff,p);
            strcat(E_buff,"\n");
            strcat(E_buff,val["data"].asString().c_str());

            val["data"]=E_buff;
            //人不再线
            if(Childthread::m_mysql->row == 0)
            {
                cout<<"no talker"<<endl;
                Childthread::m_mysql->insert("offline",val["name"].asString().c_str(),val["data"].asString().c_str());
            
                return;    
            }

            int t_fd = atoi(Childthread::m_mysql->row[1]);
            if(-1 == send(t_fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
            {
                cout<<"talk send error"<<endl; 
            }
        }
        //pthread_mutex_unlock(&mutex);
}

void Talkgroup_view::process(int &fd,char *buff)
{
    Json::Value val;
    Json::Reader read;

    if(-1 == read.parse(buff,val))
    {
        cerr<<"read faile errno"<<errno<<endl;
        return;    
    }
    
    //Childthread::m_mysql->mysql_connect();
    
    //pthread_mutex_unlock(&mutex);
    char query[128] = "";
    sprintf(query,"select * from online");
    int flag = mysql_real_query(Childthread::m_mysql->mysql,query,(unsigned int)strlen(query));
    if(flag)
    {
        cout<<"query failed!"<<endl;    
    }
    else
    {
        Childthread::m_mysql->res = mysql_store_result(Childthread::m_mysql->mysql);
        
        while(Childthread::m_mysql->row = mysql_fetch_row(Childthread::m_mysql->res))
        {
            if(Childthread::m_mysql->row == 0)
            {
                cout<<"no talker"<<endl;
                return;    
            }

            int t_fd = atoi(Childthread::m_mysql->row[1]);
            if(-1 == send(t_fd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
            {
                cout<<"talk send error"<<endl; 
            }
        }      
    }
    //pthread_mutex_unlock(&mutex);
}

