#include "transfer.h"
#include "error.h"
#include "file.h"

char *ip_addr = "192.168.2.158";
unsigned short port = 6260;
unsigned short data_port = 6160;
char *file_name = "test.txt";


void main_thread(void *args);
int main(int argc, char *argv[])
{
    t_thread t_main;
    int a;
    create_thread(&t_main, main_thread, NULL);
    for(;;)
    {
        a++;
    }

}

void main_thread(void *args)
{
    
    char *file_path = "files/";

    transfer_session session;
    transfer_frame *f_send = NULL, *f_recv = NULL;
    frame_header *f_header = NULL, *recv_header = NULL;
    login_frame *f_logini = NULL;
    file_desc *f_desc = NULL;

    t_thread t_handle[THREAD_COUNT];
    t_thread t_send, t_recv;

    q_msg pkg_msg;

    char *login_buf;

    unsigned char *buf = NULL;
    unsigned char buf_send[BUF_SIZE];
    unsigned char buf_file_name[FILE_NAME_MAX];
    unsigned char *buf_file_info;


    int len = 0, data_len = 0, frame_len = 0;
    unsigned short recv_len = 0;
    int buf_offset = 0;
    int fd;
    int code = 0;
    unsigned char file_name_len = 0, t_count = 0;
    
    init_socket(&session, ip_addr, port, data_port);


    for(;;)
    {
        //printf("Enter string to send:");
        /*
        if (buf)
        {
            printf("free ptr:%p\n", buf);
            t_free(buf);
        }
        */
        if(recv_msg_q(session.package_qid, &pkg_msg, sizeof(q_msg),\
                    MSG_TYPE_PACKAGE, IPC_NOWAIT) < 0)
            buf = NULL;
        else
            buf = *((unsigned char **)(pkg_msg.msg_buf));
            
        switch (session.state)
        {
        case STATE_WAIT_CONN:
            if(session.connect(&session) < 0)
            {
                t_log("connect to server failed!");
                DELAY(CONNECT_DELAY_SECONDS);
                break;
            }
            
            create_thread(&t_recv, receive_handler, &session);
            session.state = STATE_CONNECTED;
            //session.state = STATE_LOGIN;
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
            if((login_buf = (char *)t_malloc(1 + session.train_no_len)) == NULL) 
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

            session.state = STATE_LOGIN_SENT;
            break;

        case STATE_LOGIN_SENT:
            /* waiting for login confirm frame */
            //if ((recv_len = recv_frame(session.fd, buf)) <= 0)
            //    goto re_conn;
            if(buf)
                recv_header = (frame_header *)buf;
            else
                goto re_conn;
            if (recv_header->type != FRAME_TYPE_CONTROL ||\
                recv_header->sub_type != FRAME_CONTROL_LOGIN_CONFIRM)
            {
                t_log("receive error");
                goto re_conn;
            }

            session.state = STATE_LOGIN;
            printf("login complete!\n");
re_conn:
            if(f_header)
            {
                t_free(f_header);
                f_header = NULL;
            }
            if(login_buf)
            {
                t_free(login_buf);
                login_buf = NULL;
            }
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

            /* send file info frame */
            len = send(session.fd, buf_send, frame_len, 0);
            session.state = STATE_FILE_INFO_SENT;
            break;
        case STATE_FILE_INFO_SENT:
            /* wait for server frame */
            //if ((recv_len = recv_frame(session.fd, buf)) <= 0)
            //    goto re_login;
            if(buf)
                recv_header = (frame_header *)buf;
            else
                break;

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
                /* init send msg queue */
                f_desc->qid = create_msg_q();
                /* init file descritpor */
                init_send_file(f_desc);
                session.f_desc = f_desc;
                /* init send thread */
                create_thread(&t_send, send_thread, &session);
                /* init read threads */
                for(t_count = 0; t_count < THREAD_COUNT; t_count++)
                    create_thread(&t_handle[t_count], read_thread, &session);
                session.state = STATE_TRANSFER;
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
            //printf("transfering\n");
            break;
        case STATE_TRANSFER_FIN:
            destroy_msg_q(session.f_desc->qid);
            session.f_desc->qid = -1;
            printf("transfer finished\n");
            return;
            break;
        default:
            break;
        }
    }

}
