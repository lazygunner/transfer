#include "transfer.h"

char *g_train_no = "abcd1234";

int init_socket(transfer_session *request, char *host_ip,\
                    unsigned short control_port, unsigned short data_port)
{
    request->connect = connect_server;
    request->control_port = control_port;
    request->data_port = data_port;
    request->remote_addr = host_ip;
    request->state = STATE_WAIT_CONN;
    request->fd = 0;
    request->train_no = g_train_no;
    request->train_no_len = strlen(g_train_no);
}

int connect_server(transfer_session *request)
{
    int client_fd, data_fd;
    int len = 0;
    struct sockaddr_in *remote_addr;

    memset(&remote_addr, 0, sizeof(remote_addr));

    remote_addr = &(request->remote_con_addr);

    remote_addr->sin_family = AF_INET;
    inet_pton(AF_INET, request->remote_addr, &(remote_addr->sin_addr.s_addr));
    remote_addr->sin_port = HTONS(request->control_port);

    if((client_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return -1;
    }
    if(connect(client_fd, (struct sockaddr *)remote_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("connect");
        return -1;
    }
    request->fd = client_fd;
    printf("connected to server\n");

    remote_addr = &(request->remote_data_addr);
    remote_addr->sin_family = AF_INET;
    inet_pton(AF_INET, request->remote_addr, &(remote_addr->sin_addr.s_addr));
    remote_addr->sin_port = HTONS(request->data_port);

    if((data_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        return -1;
    }
    request->data_fd = data_fd;
   
    return client_fd;

}

int send_file_data(transfer_session *session, char *buf_send, int data_len)
{
    int send_len;
    socklen_t len;

    len = sizeof(struct sockaddr);
    send_len = sendto(session->data_fd, buf_send, data_len, 0,\
                (struct sockaddr*)&(session->remote_data_addr), len);

    return send_len;
}












