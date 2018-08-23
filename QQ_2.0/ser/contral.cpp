#include"ser.h"

const int TYPE_LOGIN = 1;
const int TYPE_REGISTER = 2;
const int TYPE_EXIT = 3;
const int TYPE_GETLIST = 4;
const int TYPE_TALKONE = 5;
const int TYPE_TALKGROUP = 6;


Contral::Contral()
{
    cout<<"contral()"<<endl;

    _model.insert(make_pair(TYPE_LOGIN,new Login_view()));
    _model.insert(make_pair(TYPE_REGISTER,new Register_view()));
    _model.insert(make_pair(TYPE_EXIT,new Exit_view()));
    _model.insert(make_pair(TYPE_GETLIST,new Getlist_view()));
    _model.insert(make_pair(TYPE_TALKONE,new Talkone_view()));
    _model.insert(make_pair(TYPE_TALKGROUP,new Talkgroup_view()));

}

Contral::~Contral()
{


}

void Contral::process(int &fd,char *buff)
{
    cout<<"contral::process"<<endl;
    
    //cout<<"con "<<&con<<endl;
	
    //解析   buff     ->type   
    Json::Value val;
    Json::Reader read;
    cout<<"1"<<endl;
    if(-1 == read.parse(buff,val))
    {
        cerr<<"read fail errno"<<errno<<endl;
        return;
    }
    
    cout<<"1"<<endl;
    int type = atoi((char*)val["type"].toStyledString().c_str());
    
    cout<<"type: "<<type<<endl;
	
    cout<<"model type"<<_model[type]<<endl;

    _model[type]->process(fd,buff);
}
