#ifndef _VIEW_H
#define _VIEW_H

class view
{
public:
    virtual void process(int &fd,char *buff)=0; 
};

#endif
