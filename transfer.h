#include <stdio.h>
#include <string.h>
#include "log.h"
#ifdef VXWORKS
#else
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define BUF_SIZE 1024
#define CONNECT_DELAY_SECONDS 10


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
    int fd;
    unsigned char train_no_len;
    unsigned char *train_no;

    
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

/* frame header */
typedef struct frame_header_tag{
    unsigned short length;
    unsigned char type;
    unsigned char sub_type;
    unsigned short crc;
}frame_header;

/* transfer frame */
typedef struct transfer_frame_tag{
    frame_header header;
    unsigned char *data;
    unsigned char tail;
}transfer_frame;

/* login frame */
typedef struct login_frame_tag{
//    unsigned int client_ip;
    unsigned char train_no_len;
    unsigned char *train_no;
}login_frame;

/* Transfer frame type & sub type */
#define FRAME_TYPE_HEARTBEAT    0X00
#define FRAME_TYPE_CONTROL      0x01
#define FRAME_TYPE_DATA         0X02

#define FRAME_CONTROL_LOGIN            0X01
#define FRAME_CONTROL_LOGIN_CONFIRM    0X02 
#define FRAME_CONTROL_CONTINUE         0X03
#define FRAME_CONTROL_FILE_INFO        0X04
#define FRAME_CONTROL_DOWNLOAD         0X05

#define FRAME_DATA_MONITOR      0X01

#define FRAME_TAIL              0xFF


/* Transfer states */
#define STATE_WAIT_CONN         0x00010000
#define STATE_CONNECTED         0x00010001
#define STATE_LOGIN             0x00010002
#define STATE_TRANSFER          0x00010003

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

unsigned short get_crc_code(const char *buf, unsigned int len);
#ifdef VXWORKS
    #define DELAY(seconds) 
    #define HTONS(host) host
    #define HTONL(host) host
#else
    #define DELAY(seconds) sleep(seconds)
    #define HTONS(host) htons(host)
    #define HTONL(host) htonl(host)
#endif

#define t_malloc(x) malloc(x)
#define t_free(x) free(x)

