#ifndef _SER_H
#define _SER_H
#include<iostream>
#include<unistd.h>
#include<assert.h>
#include<pthread.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string>
#include<event.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<json/json.h>
#include<map>
#include"view.h"
#include"contral.h"
#include"Func_View.h"
#include<mysql/mysql.h>
#include<errno.h>
#include<vector>
#include<memory>
using namespace std;

class Childthread;
class Mysql;


//服务器
class Ser
{
public:
    //class Ser;

    static Ser* Getser()
    {
        return &_ser;    
    }

    std::map<int,int> _pth_count;//线程监听量  <s1,int>
	std::map<int,Childthread*> _childpth;//子线程
    std::vector<int[2]> _socketpair;//双向管道	

private:
    Ser();
    ~Ser();
    
    static Ser _ser;    

    struct event_base *_base;//libevent
};

void listen_cb(int fd, short event, void* arg);

void *thread_fun(void *arg);

//子线程
class Childthread
{
public:
    Childthread(int s1);
    ~Childthread();
    static Contral* con;
	static Mysql* m_mysql;
    
    std::map<int,struct event*> _event_map;//客户端套接字事件
private:
    
    struct event_base *_base;
//    friend void func_cb(int fd, short event, void* arg);
  //  friend void cli_cb(int fd, short event, void *arg);
};
void func_cb(int fd, short event, void* arg);
void cli_cb(int fd, short event, void *arg);

/*
	void mysql_connect();
    bool mysql_insert(const char* table, const char *name, const char *pw);
	bool mysql_search(const char* table, const char *name, const char *pw);
	bool mysql_delete(char const* table, char const* s1, const char* s2);
*/


class Mysql
{
public:
	Mysql(const char* username, const char* pw);
	~Mysql();
    
	void mysql_connect();
    bool insert(const char* table, const char *name, const char *pw);
	bool search(const char* table, const char *name, const char *pw);
	bool mydelete(char const* table, char const* s1, const char* s2);
	
    MYSQL mm;

    MYSQL *mysql;
	MYSQL_RES *res;//接收指令的返回值
	MYSQL_ROW row;//获取返回值的一行
};


#endif
