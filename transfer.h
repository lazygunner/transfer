#ifndef _TRANS_H_
#define _TRANS_H_

#define DEBUG
#undef DEBUG


#include <stdio.h>
#include <string.h>
#include "log.h"
#include "thread.h"
#ifdef VXWORKS
#else
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>

    #include <sys/ipc.h>
    #include <sys/msg.h>
    #include <signal.h>
#endif

#define BUF_SIZE 1024
#define FILE_NAME_MAX 256
#define CONNECT_DELAY_SECONDS 10
#define MAX_WAIT_SECONDS 120
#define LEN_HEADER 2
#define THREAD_COUNT 3

#define TRAIN_NO_LEN            20
#define FRAME_DATA_LEN          2048
#define HEARTBEAT_LEN           7

#define ORIGIN_FRAME            0x00
#define RETRAN_FRAME            0x01

typedef long off_t;




/* used especially for retry frames */
typedef struct frame_index_tag frame_index;
struct frame_index_tag
{
    unsigned short  block_index;
    unsigned short  frame_index;
    frame_index     *next;
};
 
/*
        ----        ----
       |blk |      |blk |
       |desc|------|desc|---....
        ----        ----
         |__ ----    |_______ ----      ----
            |b_no|           |b_no|    |b_no|
            |FFFF|           |f_no|----|f_no|---....
             ----             ----      ----
*/
typedef struct file_block_desc_tag file_block_desc;
struct file_block_desc_tag
{
    unsigned char   retry_flag;
    frame_index     *index;
    file_block_desc *next;
};
 
 
typedef struct file_frame_data_tag
{
    unsigned short  file_id;
    unsigned short  block_index;
    unsigned short  frame_index;
    unsigned char   data[FRAME_DATA_LEN];
}file_frame_data;

typedef struct file_desc
{
    unsigned char   *file_name;
    int             file_size;      /* max file size < 2GB */
    unsigned short  file_id;
    file_block_desc *block_head;
    file_block_desc *block_tail;
    t_lock          *block_list_lock;
    unsigned short  block_count;
    unsigned short  last_frame_count;
    unsigned short  *frame_remain;
}file_desc;

typedef struct data_msg_tag
{
    unsigned char   *data;
    unsigned short  data_len;
}data_msg;

typedef struct q_msg_tag
{
    unsigned int    msg_type;
    data_msg        msg_buf;
}q_msg;

/* frame header */
typedef struct frame_header_tag{
    unsigned short length;
    unsigned char type;
    unsigned char sub_type;
    unsigned short crc;
}frame_header;

/* sent block frame */
typedef struct block_sent_frame_tag{
    frame_header    f_header;
    unsigned short  file_id;
    unsigned short  block_index;
    unsigned char   f_tail;
}__attribute__ ((__packed__)) block_sent_frame;

/* receive finish frame */
typedef struct finish_frame_tag{
    frame_header    f_header;
    unsigned short  file_id;
    unsigned char   f_tail;
}__attribute__ ((__packed__)) finish_frame;

typedef struct transfer_session_tag transfer_session;
struct transfer_session_tag
{
    //info reserve
    char *hostname;
    char *username;
    char *password;
    char *directory;


    unsigned int control_port;
    unsigned int data_port;
    int state;
    int fd;
    int data_fd;
    unsigned char train_no_len;
    unsigned char *train_no;

    unsigned char *hb;

    void *remote_addr;
    size_t remote_addr_len;
    struct sockaddr_in remote_con_addr;
    struct sockaddr_in remote_data_addr;
    int ai_family;
    
    int package_qid;
    int data_qid;


    void *protocol_data;

    file_desc *f_desc;

    int (* init) (transfer_session *session);
    size_t (* read_function) (transfer_session *session, 
                                void *ptr,
                                size_t size,
                                int fd);
    size_t (* write_function) (transfer_session *session, 
                                void *ptr,
                                size_t size,
                                int fd);

    int (* connect) (transfer_session *session);
    int (* disconnect) (transfer_session *session);

    off_t (* transfer_file);


};


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
#define FRAME_TYPE_MAX          0X03

#define FRAME_CONTROL_LOGIN             0X01
#define FRAME_CONTROL_LOGIN_CONFIRM     0X02 
#define FRAME_CONTROL_CONTINUE          0X03
#define FRAME_CONTROL_FILE_INFO         0X04
#define FRAME_CONTROL_DOWNLOAD          0X05
#define FRAME_CONTROL_ERROR             0X06
#define FRAME_CONTROL_SENT              0X07
#define FRAME_CONTROL_FINISHED          0X08

#define FRAME_DATA_MONITOR              0X01

#define FRAME_SUB_TYPE_MAX              0X07

#define FRAME_TAIL              0xFF

/* Msg Type */
#define MSG_TYPE_PACKAGE        0x01
#define MSG_TYPE_FILE_FRAME     0x02

/* Msg queue key id */
#define MSG_Q_KEY_ID_RECV       0x01
#define MSG_Q_KEY_ID_DATA       0x02

/* Transfer states */
#define STATE_WAIT_CONN         0x00010000
#define STATE_CONNECTED         0x00010001
#define STATE_LOGIN_SENT        0x00010002
#define STATE_LOGIN             0x00010003
#define STATE_FILE_INFO_SENT    0x00010004
#define STATE_TRANSFER          0x00010005
#define STATE_TRANSFER_FIN      0x00010006
#define STATE_CONN_LOST         0x00010007


int connect_server(transfer_session *session);

unsigned short get_crc_code(const unsigned char *buf, unsigned int len);
#ifdef VXWORKS
    #define DELAY(seconds) 
    #define HTONS(host) host
    #define HTONL(host) host
    #define NTOHS(host) host
    #define NTOHL(host) host
#else
    #define DELAY(seconds) sleep(seconds)
    #define HTONS(host) htons(host)
    #define HTONL(host) htonl(host)
    #define NTOHS(host) ntohs(host)
    #define NTOHL(host) ntohl(host)

#endif


#endif
