#include "transfer.h"

char *ip_addr = "192.168.2.158";
unsigned short port = 8877;



int main(int argc, char *argv[])
{
    transfer_request request;
    transfer_frame *frame_send, *frame_recv;
    char buf[BUF_SIZE];
    int len = 0;
    int fd;
    int code = 0;
    
    init_socket(&request, ip_addr, port);


    sock = request.connect(&request);
    if (sock < 0)
        return;

    for(;;)
    {
        //printf("Enter string to send:");
        switch (request.state)
        {
        case STATE_WAIT_CONN:
            if((request.fd = request.connect(&request)) < 0)
            {
                DELAY(CONNECT_DELAY_SECONDS);
                break;
            }

            request.state = STATE_CONNECTED;
            break;
        case STATE_CONNECTED:
            if (request.fd < 0)
            {
                log("connection socket lost!");
                request.state = STATE_WAIT_CONN;
                break;
            }
            frame_send = frame_init()
            

        /* |TYPE|LEN|CODE|CRC|DATA               |  */
        case STATE_LOGIN:
            len = recv(request.fd, buf, BUF_SIZE, 0);
            cmd = server_msg_check(buf, len);

            switch (cmd)
            {
            case CMD_CHECK_FILE:
                
            case CMD_TRASFER:

            }
            break;
        case STATE_TRANSFER:
            break;
        default:
            break;
        }
    }


        scanf("%s", &buf);
  
        if(!strcmp(buf, "quit"))
            break;
 
        len = send(sock, buf, strlen(buf), 0);
        len = recv(sock, buf, BUF_SIZE, 0);
 
        buf[len] = '\0';
 
        printf("received:%s\n", buf);
 
    }





}
