#include "transfer.h"

char *g_train_no = "abcd1234";

int init_socket(transfer_session *request,\
                    unsigned short host_port, char *host_ip)
{
    request->connect = connect_server;
    request->port = host_port;
    request->remote_addr = host_ip;
    request->state = STATE_WAIT_CONN;
    request->fd = 0;
    request->train_no = g_train_no;
    request->train_no_len = strlen(g_train_no);
}

int connect_server(transfer_session *request)
{
    int client_fd;
    int len = 0;
    struct sockaddr_in remote_addr;

    memset(&remote_addr, 0, sizeof(remote_addr));

    remote_addr.sin_family = AF_INET;
    //remote_addr.sin_addr.s_addr = inet_addr(request->remote_addr);
    inet_pton(AF_INET, request->remote_addr, &remote_addr.sin_addr.s_addr);
    remote_addr.sin_port = HTONS(request->port);
    if((client_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return -1;
    }
    if(connect(client_fd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("connect");
        return -1;
    }

    printf("connected to server\n");

    return client_fd;

}














