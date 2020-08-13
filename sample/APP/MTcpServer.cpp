#include "MTcpServer.h"
#include <memory.h>
extern "C"{
#include <ak_log.h>
#include <ak_common.h>
}

MTcpServer::MTcpServer(/* args */)
{
    if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        ak_print_error_ex(MODULE_ID_LOG, "create socket error\n");
        return;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8000);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        ak_print_error_ex(MODULE_ID_LOG, "bind socket error\n");
    }
    if( listen(listenfd, 10) == -1){
        ak_print_error_ex(MODULE_ID_LOG, "listen socket error\n");
        return;
    }
}

MTcpServer::~MTcpServer()
{
    close(listenfd);
}

void MTcpServer::run()
{
    ak_print_normal(MODULE_ID_LOG, "waitting for client's request\n");

    while (1)
    {
        if ( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
            continue;
        }
        break;
    }
    while(1){
        n = recv(connfd, buff, MAXLINE, 0);
        dbf->rb_write(buff, n);
    }
    close(connfd);
}

static void * callback(void *arg)
{
    ((MTcpServer*)arg)->run();
}

void MTcpServer::start()
{
    BaseThread::start(callback);
}