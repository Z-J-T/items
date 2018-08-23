#include"cli.h"

const char* IP = "127.0.0.1";

const int SER_PORT = 8000;

int main()
{
	//链接服务器
    int fd = socket(AF_INET,SOCK_STREAM,0);
    assert(fd != -1);

    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(SER_PORT);
	
    int res = connect(fd,(struct sockaddr*)&saddr,sizeof(saddr));
    
    assert(res != -1);
	run(fd);
	
    return 0;
}
