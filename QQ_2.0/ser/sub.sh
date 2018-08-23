#!/bin/bash

g++ -o ser ser.cpp mysql_API.cpp reactor.cpp Func_View.cpp contral.cpp  -lpthread -ljson -levent -L /usr/lib/mysql -lmysqlclient -g 



#-L /usr/lib/mysql/ -lmysqlclient -g

