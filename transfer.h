#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

typedef long off_t;

typedef struct transfer_request_tag transfer_request;
struct transfer_request_tag
{
    //info reserve
    char *hostname;
    char *username;
    char *password;
    char *directory;


    unsigned int port;
    int state;
    int datafd;

    
    void *remote_addr;
    size_t remote_addr_len;
    int ai_family;

    void *protocol_data;

    int (* init) (transfer_request *request);
    size_t (* read_function) (transfer_request *request, 
                                void *ptr,
                                size_t size,
                                int fd);
    size_t (* write_function) (transfer_request *request, 
                                void *ptr,
                                size_t size,
                                int fd);

    int (* connect) (transfer_request *request);
    int (* disconnect) (transfer_request *request);

    off_t (* transfer_file);


};

/* Transfer states */
#define STATE_WAIT_CONN         0x00010000
#define STATE_CONNECTED         0x00010001
#define STATE_TRANSFER          0x00010002

/* Error types */
#define ERROR_RETRYABLE         0x80000001   /* Temporary failure. The GUI
                                               should wait briefly */
#define ERROR_FATAL             0x80000002   /* Fatal error */
#define ERROR_RETRYABLE_NO_WAIT 0x80000003   /* Temporary failure. The GUI
                                               should not wait and should
                                               reconnect */
#define ERROR_NOTRANS           0x80000004   /* Custom error. This is
                                               returned when a FXP transfer
                                               is requested */
#define ERROR_TIMEDOUT          0x80000005   /* Connected timed out */

int connect_server(transfer_request *request);
