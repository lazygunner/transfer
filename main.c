#include "transfer.h"

char *ip_addr = "192.168.2.158";

int main(int argc, char *argv[])
{
    transfer_request request;
    char buf[BUF_SIZE];
    int len = 0;
    int sock;

    request.connect = connect_server;
    request.port = 8877;
    request.remote_addr = ip_addr;

    sock = request.connect(&request);
    if (sock < 0)
        return;

    while(1)
    {
        printf("Enter string to send:");
        scanf("%s", &buf);
  
        if(!strcmp(buf, "quit"))
            break;
 
        len = send(sock, buf, strlen(buf), 0);
        len = recv(sock, buf, BUF_SIZE, 0);
 
        buf[len] = '\0';
 
        printf("received:%s\n", buf);
 
    }





}
