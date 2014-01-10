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
    request->package_qid = create_msg_q(MSG_Q_KEY_ID_RECV);
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

void receive_handler(void *args)
{
    fd_set              rset;
    int                 sock_fd;
    int                 recv_len;
    int                 frame_len;
    int                 len;
    int                 data_len;
    unsigned short      recv_crc;
    unsigned char       buf[1024];
    unsigned char       *handle_package;
    q_msg               package_msg;
    transfer_session    *session;
    struct timeval timeout={1,0};

    session = (transfer_session *)args;
    sock_fd = session->fd;

    FD_ZERO(&rset);
    for(;;)
    {
        FD_SET(sock_fd, &rset);
        select(sock_fd + 1, &rset, NULL, NULL, NULL);
        if(FD_ISSET(sock_fd, &rset))
        {
            //printf("fd is set!");
            len = recv(sock_fd, buf, LEN_HEADER, 0);
            
            recv_len = NTOHS(*((unsigned short *)buf));
            if (recv_len > 100 || recv_len <= 0)
            {
                t_log("receive package length error!");
                continue;
            }
            frame_len = recv(sock_fd, buf + LEN_HEADER, recv_len, 0);
            
            if (frame_len != recv_len)
                continue;
            frame_len += LEN_HEADER;
               
            if (buf[recv_len + 1] != FRAME_TAIL)
                continue;
             
            data_len = recv_len + 1 - sizeof(frame_header);
                     
            recv_crc = get_crc_code(buf + sizeof(frame_header), data_len);
               
            if(recv_crc != NTOHS(((frame_header *)buf)->crc))
            {
                t_log("receive crc error");
                continue;
            }

            handle_package = (unsigned char *)t_malloc(frame_len);
            //printf("malloc ptr:%p len:%d\n", handle_package, frame_len);
            memcpy(handle_package, buf, frame_len);
            memcpy(package_msg.msg_buf, &handle_package,\
                    sizeof(handle_package));
            package_msg.msg_type = MSG_TYPE_PACKAGE;
            
            //printf("send to msg q\n");
            if(send_to_msg_q(session->package_qid, &package_msg,\
                                    sizeof(q_msg), 0) < 0)
            {
                perror("msg closed! quit the system!");
            }

            //printf("after send to msg q\n");
        }


    }


}










