#include "transfer.h"
#include "error.h"
#include "file.h"

char *ip_addr = "192.168.2.158";
unsigned short port = 8877;
char *file_name = "test.txt";
char *file_path = "files/";


int main(int argc, char *argv[])
{
    transfer_session session;
    transfer_frame *f_send, *f_recv;
    frame_header *f_header, *recv_header;
    login_frame *f_login;
    

    file_desc *f_desc;

    char *login_buf;

    unsigned char buf[BUF_SIZE];
    unsigned char buf_send[BUF_SIZE];
    unsigned char *buf_file_info;


    int len = 0, login_len = 0, file_info_len = 0, file_count = 0;
    unsigned short recv_len = 0;
    int buf_offset = 0;
    int fd;
    int code = 0;
    
    init_socket(&session, port, ip_addr);


    for(;;)
    {
        //printf("Enter string to send:");
        switch (session.state)
        {
        case STATE_WAIT_CONN:
            if((session.fd = session.connect(&session)) < 0)
            {
                t_log("connect to server failed!");
                DELAY(CONNECT_DELAY_SECONDS);
                break;
            }

            session.state = STATE_CONNECTED;
            break;
        case STATE_CONNECTED:
            if (session.fd < 0)
            {
                t_log("connection socket lost!");
                goto re_conn;
            }
            if((f_header = (frame_header *)t_malloc(sizeof(f_header))) == NULL) 
            {
                t_log("malloc frame header error!");
                goto re_conn;
            }
            if(frame_header_init(FRAME_TYPE_CONTROL, FRAME_CONTROL_LOGIN,\
                    f_header) < 0)
            {
                t_log("frame init failed!");
                t_free(f_header);
                goto re_conn;
            }
            /*
            if((f_login = (login_frame *)t_malloc(sizeof(login_frame)))\
                == NULL)
            {
                t_log("malloc login frame error!");
                t_free(f_send);
                goto re_conn;
            }

            f_login->car_no = TEST_CAR_NO;
            f_login->car_no_len = sizeof(TEST_CAR_NO);
            */
            if((login_buf = (char *)malloc(1 + session.train_no_len)) == NULL) 
            {
                t_log("malloc login_buf frame error!");
                t_free(f_header);
                goto re_conn;
            }

            memset(login_buf, session.train_no_len, 1);
            memcpy(login_buf + 1, session.train_no, session.train_no_len);
            
            login_len = session.train_no_len + 1;
            f_header->length = sizeof(f_header) + login_len + 1;
            /* crc of the data section */
            f_header->crc = get_crc_code(login_buf, login_len);

            memcpy(buf_send, f_header, sizeof(frame_header));
            buf_offset += sizeof(frame_header);
            memcpy(buf_send + buf_offset, login_buf, session.train_no_len);
            buf_offset += session.train_no_len;
            memset(buf_send + buf_offset, FRAME_TAIL, 1);
            /* send login frame */
            len = send(session.fd, buf_send, f_header->length, 0);
            /* waiting for login confirm frame */
            memset(buf, 0, 1024);
            len = recv(session.fd, buf, LEN_HEADER, 0);
            recv_len = HTONS(*((unsigned short *)buf));
            if (recv_len > 100 || recv_len <= 0)
            {
                t_log("receive package length error!");
                t_free(f_header);
                t_free(login_buf);
                goto re_conn;
            }
            recv(session.fd, buf + 2, recv_len, 0);
            recv_header = (frame_header *)buf;
            if (buf[recv_len + 1] != FRAME_TAIL || recv_header->type != FRAME_TYPE_CONTROL || recv_header->sub_type != FRAME_CONTROL_LOGIN_CONFIRM)
            {
                t_log("receive error");
                t_free(f_header);
                t_free(login_buf);
                goto re_conn;
            }
            session.state = STATE_LOGIN;
            printf("connect complete!");
            t_free(f_header);
            t_free(login_buf);
            break;
            
        /* |TYPE|LEN|CODE|CRC|DATA               |  */
        case STATE_LOGIN:

            file_info_len = get_file_info(file_path, &buf_file_info, &file_count);
            memset(buf, 0, 1024);
            len = recv(session.fd, buf, LEN_HEADER, 0);
            recv_len = HTONS(*((unsigned short *)buf));
            if (recv_len > 100 || recv_len <= 0)
            {
                t_log("receive package length error!");
                goto re_conn;
            }
            recv(session.fd, buf + 2, recv_len, 0);
            recv_header = (frame_header *)buf;
            if (buf[recv_len + 1] != FRAME_TAIL || recv_header->type != FRAME_TYPE_CONTROL)
            {
                t_log("receive error");
                break;
            }
            /* handle the sub type */
            switch(recv_header->sub_type)
            {
            case FRAME_CONTROL_DOWNLOAD:
                if(NULL == (f_desc =\
                    (file_desc *)t_malloc(sizeof(file_desc))));
                {
                    t_log("malloc login_buf frame error!");
                    break;
                }
                f_desc->file_name = file_name;
                init_send_file(f_desc);
                break;
            case FRAME_CONTROL_CONTINUE:
                break;
            default:
                break;
            }

            break;
        case STATE_TRANSFER:
            break;
        default:
re_conn:
            session.state = STATE_WAIT_CONN;
            break;
        }
    }

}
