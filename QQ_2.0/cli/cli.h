#ifndef CLI_H
#define CLI_H

#include<iostream>
#include<json/json.h>
#include<errno.h>
#include<stdbool.h>
#include<sys/stat.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<pthread.h>

using namespace std;

void run(int fd);

void Login(int fd);

void login_success(int fd);

void Register(int fd);

void Exit(int fd);

void Get_List(int fd);

void Talk_One(int fd);

void Talk_Group(int fd);

void L_Exit(int fd);

void *thread_recv(void* arg);

#endif // !CLI_H

