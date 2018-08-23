#ifndef CONTRAL_H
#define CONTRAL_H
#include"ser.h"
#include<iostream>
using namespace std;

//class contral;

class Contral
{
public:
    
    std::map<const int,view*> _model;//继承和多态


	Contral();
	~Contral();
    
	void process(int &fd,char* buff);

};


#endif
