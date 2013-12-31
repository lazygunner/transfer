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
    unsigned char buf_file_name[FILE_NAME_MAX];
    unsigned char *buf_file_info;


    int len = 0, data_len = 0, frame_len = 0;
    unsigned short recv_len = 0;
    int buf_offset = 0;
    int fd;
    int code = 0;
    unsigned char file_name_len = 0;
    
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
            
            /* init frame header area */
            if(frame_header_init(FRAME_TYPE_CONTROL, FRAME_CONTROL_LOGIN,\
                    &f_header) < 0)
            {
                t_log("frame init failed!");
                goto re_conn;
            }
            
            /* init frame data area */
            if((login_buf = (char *)malloc(1 + session.train_no_len)) == NULL) 
            {
                t_log("malloc login_buf frame error!");
                goto re_conn;
            }
            memset(login_buf, session.train_no_len, 1);
            memcpy(login_buf + 1, session.train_no, session.train_no_len);
            data_len = session.train_no_len + 1;
            
            /* build sending frame */
            frame_len = frame_build(f_header, login_buf, data_len, buf_send);

            /* send login frame */
            len = send(session.fd, buf_send, frame_len, 0);

            /* waiting for login confirm frame */
            if ((recv_len = recv_frame(session.fd, buf)) <= 0)
                goto re_conn;

            recv_header = (frame_header *)buf;
            if (recv_header->type != FRAME_TYPE_CONTROL ||\
                recv_header->sub_type != FRAME_CONTROL_LOGIN_CONFIRM)
            {
                t_log("receive error");
                goto re_conn;
            }

            session.state = STATE_LOGIN;
            printf("login complete!");
re_conn:
            if(f_header)
                t_free(f_header);
            if(login_buf)
                t_free(login_buf);
            break;
            
        case STATE_LOGIN:

            /* init frame header area */
            if(frame_header_init(FRAME_TYPE_CONTROL, FRAME_CONTROL_FILE_INFO,\
                    &f_header) < 0)
            {
                t_log("frame init failed!");
                goto re_login;
            }
            
            /* init data area */
            data_len = get_file_info(file_path, &buf_file_info);

            /* build frame */
            frame_len = frame_build(f_header, buf_file_info, data_len, buf_send);

            /* send login frame */
            len = send(session.fd, buf_send, frame_len, 0);
            
            /* wait for server frame */
            if ((recv_len = recv_frame(session.fd, buf)) <= 0)
                goto re_login;

            recv_header = (frame_header *)buf;
            if (recv_header->type != FRAME_TYPE_CONTROL)
            {
                t_log("receive error");
                goto re_login;
            }
            /* handle the sub type */
            switch(recv_header->sub_type)
            {
            case FRAME_CONTROL_DOWNLOAD:
                if(NULL == (f_desc = (file_desc *)t_malloc(sizeof(file_desc))))
                {
                    t_log("malloc login_buf frame error!");
                    goto re_login;
                }
                
                file_name_len = *(buf + sizeof(frame_header) + 1);
                memcpy(buf_file_name + strlen(strcpy(buf_file_name,\
                        file_path)), buf + sizeof(frame_header) + 2,\
                        file_name_len);

                f_desc->file_id = *((unsigned short *)(\
                        buf + sizeof(frame_header) + 2 + file_name_len));
                f_desc->file_name = buf_file_name;
                init_send_file(f_desc);
                break;
            case FRAME_CONTROL_CONTINUE:
                break;
            default:
                break;
            }

re_login:
            if(f_header)
                t_free(f_header);
            if(buf_file_info)
                t_free(buf_file_info);

            break;
        case STATE_TRANSFER:
            break;
        default:
            break;
        }
    }

}
