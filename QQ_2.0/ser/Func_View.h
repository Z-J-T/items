
#ifndef LOGIN_VIEW_H
#define LOGIN_VIEW_H

#include"ser.h"

class Login_view:public view
{
public:
    Login_view(){}
    ~Login_view(){}
	void process(int &fd,char* buff);
};

class Register_view:public view
{
public:
    void process(int &fd,char* buff);      
};

class Exit_view:public view
{
public:
    Exit_view(){}
    ~Exit_view(){}
    void process(int &fd,char* buff);  
};

class Getlist_view:public view
{
public:
    Getlist_view(){}
    ~Getlist_view(){}
    void process(int &fd,char* buff);    
};

class Talkone_view:public view
{
public:
    Talkone_view(){}
    ~Talkone_view(){}
    void process(int &fd,char* buff);   
};

class Talkgroup_view:public view
{
public:
    Talkgroup_view(){}
    ~Talkgroup_view(){}
    void process(int &fd,char* buff);      
};

#endif
