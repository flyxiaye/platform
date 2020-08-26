#ifndef RUN_SIG_H
#define RUN_SIG_H

#include "BaseThread.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

enum Dir{
    MSG_OUT,
    MSG_IN
};

class RunState :public BaseThread
{
public:

    RunState(int sock, sockaddr_in *server, Dir dir);
    ~RunState();
    void start();
    void run();
    int *run_state;
private:
    struct sockaddr_in *server;
    int socket1;
    Dir dir;
    unsigned char *datbuf;
};

#endif