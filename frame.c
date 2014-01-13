#include "transfer.h"



int frame_header_init(unsigned char type, unsigned char sub_type,\
                        frame_header **header)
{
    
    if((*header = (frame_header *)t_malloc(sizeof(frame_header))) == NULL)
    {
        t_log("malloc frame header error!");
        return -1;
    }


    if(type >= 0 && type < FRAME_TYPE_MAX &&\
        sub_type >= 0 && sub_type < FRAME_SUB_TYPE_MAX)
    {

        (*header)->type = type;
        (*header)->sub_type = sub_type;
        (*header)->crc = 0;
        (*header)->length = 0;
        return 0;
    }
    else
    {
        t_log("type unknown");
        return -1;
    }

}
/* init heart beat frame */
int frame_heartbeat_init(unsigned char **heartbeat_buf)
{
    frame_header *f_header = NULL;

    frame_header_init(FRAME_TYPE_HEARTBEAT, FRAME_TYPE_HEARTBEAT, &f_header);
    f_header->length  = HTONS(5);

    heartbeat_buf = (unsigned char *)t_malloc(sizeof(frame_header) + 1);

    memcpy(heartbeat_buf, f_header, sizeof(frame_header));
    memset(heartbeat_buf + sizeof(frame_header), FRAME_TAIL, 1);

    t_free(f_header);

    return 0;

}

int frame_build(frame_header *header, unsigned char *data,\
                    unsigned int data_len, unsigned char *buf_send)
{
    int buf_offset = 0, frame_len = 0;

    frame_len = sizeof(frame_header) + data_len + 1;
    header->length = HTONS(frame_len - 2);
    /* crc of the data section */
    header->crc = HTONS(get_crc_code(data, data_len));
 
    buf_offset = 0;
    memcpy(buf_send, header, sizeof(frame_header));
    buf_offset += sizeof(frame_header);
    memcpy(buf_send + buf_offset, data, data_len);
    buf_offset += data_len;
    memset(buf_send + buf_offset, FRAME_TAIL, 1);

    return frame_len;
}

int recv_frame(int sock, unsigned char *buf)
{
    unsigned short len = 0, recv_len = 0, frame_len = 0, data_len = 0;
    unsigned short recv_crc = 0;

    if (!buf)
        return -1;
    memset(buf, 0, 1024);

    if (!sock)
        return -1;
    len = recv(sock, buf, LEN_HEADER, 0);

    recv_len = NTOHS(*((unsigned short *)buf));
    if (recv_len > 100 || recv_len <= 0)
    {
        t_log("receive package length error!");
        return -1;
    }
    frame_len = recv(sock, buf + 2, recv_len, 0);
    
    if (frame_len != recv_len)
        return -1;
    
    if (buf[recv_len + 1] != FRAME_TAIL)
        return -1;

    data_len = recv_len + 1 - sizeof(frame_header);
    
    recv_crc = get_crc_code(buf + sizeof(frame_header), data_len);

    if(recv_crc != NTOHS(((frame_header *)buf)->crc))
    {
        t_log("crc error");
        return -1;
    }

    return frame_len + 2;

}
