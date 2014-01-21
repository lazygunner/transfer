#include "transfer.h"
#include "error.h"
#include "file.h"

char *ip_addr = "192.168.2.23";
unsigned short port = 6260;
unsigned short data_port = 6160;
char *file_name = "test.txt";


void main_thread(void *args);
int main(int argc, char *argv[])
{
    t_thread t_main;
    int a;

    sigset_t signal_mask;
    sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGPIPE);
    int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
    if (rc != 0) {
           printf("block sigpipe error\n");
    } 
    mem_pool_init();
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
    finish_frame *finish = NULL;

    t_thread t_handle[THREAD_COUNT];
    t_thread t_send, t_recv, t_heartbeat;

    q_msg pkg_msg;

    char *login_buf;

    unsigned char *buf = NULL;
    unsigned char buf_send[BUF_SIZE];
    unsigned char buf_file_name[FILE_NAME_MAX];
    unsigned char *buf_file_info;
    unsigned char *buf_pos;
    unsigned char re_connect = 0;


    int len = 0, data_len = 0, frame_len = 0;
    unsigned short recv_len = 0;
    unsigned short file_id = 0;
    int buf_offset = 0;
    int fd;
    int code = 0;
    unsigned char file_name_len = 0, t_count = 0;
    int wait_seconds = MAX_WAIT_SECONDS;
    
    init_socket(&session, ip_addr, port, data_port);
    frame_heartbeat_init(&session.hb);
    if(NULL == (f_desc = init_file_desc()))
    {
        t_log("malloc f_desc frame error!");
        return;
    }
    session.f_desc = f_desc;
 
    /* create receive messsage queue */
    session.package_qid = create_msg_q(MSG_Q_KEY_ID_RECV);
    session.data_qid = create_msg_q(MSG_Q_KEY_ID_DATA);
    

    /* init send thread */
    create_thread(&t_send, send_thread, &session);
    /* init read threads */
    for(t_count = 0; t_count < THREAD_COUNT; t_count++)
        create_thread(&t_handle[t_count], read_thread, &session);


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
            buf = pkg_msg.msg_buf.data;
            
        switch (session.state)
        {
        case STATE_WAIT_CONN:
            /* establish the socket connection with server */
            if(session.connect(&session) < 0)
            {
                t_log("connect to server failed!");
                DELAY(CONNECT_DELAY_SECONDS);
                break;
            }
            session.state = STATE_CONNECTED;
            /* create receive handler thread */
            create_thread(&t_recv, receive_handler, &session);

            break;
        case STATE_CONNECTED:
            if (session.fd < 0)
            {
                t_log("connection socket lost!");
                re_connect = 1;
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
            if((len = send(session.fd, buf_send, frame_len, 0)) != frame_len)
            {
                t_log("[login]lost connection!\n");
                re_connect = 1;
                goto re_conn;
            }
            printf("login sent\n");
            session.state = STATE_LOGIN_SENT;
re_conn:    
            if(re_connect)
            {
                re_connect = 0;
                session.state = STATE_CONN_LOST;
            }
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

        
        case STATE_LOGIN_SENT:
            /* waiting for login confirm frame */
            if(buf)
            {
                recv_header = (frame_header *)buf;
                wait_seconds = MAX_WAIT_SECONDS;
            }
            else
            {
                if(wait_seconds)
                {
                    wait_seconds--;
                    sleep(1);
                    break;
                }
                else
                {
                    wait_seconds = MAX_WAIT_SECONDS;
                    session.state = STATE_CONN_LOST;
                    break;
                }
             }

            if (recv_header->type != FRAME_TYPE_CONTROL ||\
                recv_header->sub_type != FRAME_CONTROL_LOGIN_CONFIRM)
            {
                t_log("[login_sent]receive error\n");
                break;
            }

            session.state = STATE_LOGIN;
            t_log("login complete!\n");
            /* send heart beat after login confirmed */
            create_thread(&t_heartbeat, send_heartbeat_thread, &session);
            break;
            
        case STATE_LOGIN:

            /* init frame header area */
            if(frame_header_init(FRAME_TYPE_CONTROL,\
                FRAME_CONTROL_FILE_INFO, &f_header) < 0)
            {
                t_log("frame init failed!");
                goto re_login;
            }
            
            /* init data area */
            data_len = get_file_info(file_path, &buf_file_info);

            /* build frame */
            frame_len = frame_build(f_header, buf_file_info, data_len, buf_send);

            /* send file info frame */
            if ((len = send(session.fd, buf_send, frame_len, 0)) != frame_len)
            {
                t_log("[login]send file info failed.\n");
                re_connect = 1;
                goto re_login;
            }
            session.state = STATE_FILE_INFO_SENT;
re_login:
            if(re_connect)
            {
            
                printf("re conn\n");
                re_connect = 0;
                session.state = STATE_CONN_LOST;
            }
            if(f_header)
            {
                t_free(f_header);
                f_header = NULL;
            }
            if(buf_file_info)
            {
                t_free(buf_file_info);
                buf_file_info = NULL;
            }
            printf("login\n");
            break;
        case STATE_FILE_INFO_SENT:
            /* must have a timeout! */
            /* wait for server frame */
            if(buf)
            {
                recv_header = (frame_header *)buf;
                wait_seconds = MAX_WAIT_SECONDS;
            }
            else
            {
                if(wait_seconds)
                {
                    wait_seconds--;
                    sleep(1);
                    break;
                }
                else
                {
                    wait_seconds = MAX_WAIT_SECONDS;
                    session.state = STATE_CONN_LOST;
                    break;
                }
             }

            if (recv_header->type != FRAME_TYPE_CONTROL)
            {
                t_log("receive error");
                break;
            }
            /* handle the sub type */
            switch(recv_header->sub_type)
            {
            case FRAME_CONTROL_DOWNLOAD:
                file_name_len = *(buf + sizeof(frame_header) + 1);
                memset(buf_file_name, 0, FILE_NAME_MAX);
                memcpy(buf_file_name + strlen(strcpy(buf_file_name,\
                        file_path)), buf + sizeof(frame_header) + 2,\
                        file_name_len);

                file_id = HTONS(*((unsigned short *)(\
                        buf + sizeof(frame_header) + 2 + file_name_len)));
                
                printf("download %s \n", buf_file_name);
                /* init file descriptor */
                set_file_desc(f_desc, file_id, buf_file_name);
                
                /* init send file data */
                init_send_list(f_desc);
                
                session.state = STATE_TRANSFER;
                /* init send thread */
                //create_thread(&t_send, send_thread, &session);
                /* init read threads */
                //for(t_count = 0; t_count < THREAD_COUNT; t_count++)
                //    create_thread(&t_handle[t_count], read_thread, &session);
                break;
            case FRAME_CONTROL_CONTINUE:
                printf("re tran\n");
                /* save the file info */
                buf_pos = buf;
                buf_pos += sizeof(frame_header);
                file_name_len = *buf_pos;
                buf_pos += 1;
                memset(buf_file_name, 0, FILE_NAME_MAX);
                memcpy(buf_file_name + strlen(strcpy(buf_file_name,\
                        file_path)), buf_pos, file_name_len);
                
                buf_pos += file_name_len;
                file_id = NTOHS(*((unsigned short *)buf_pos));

                set_file_desc(f_desc, file_id, buf_file_name);

                /* file name len:1, file_name:n, file_id:2 */
                buf_pos += 2;

                /* skip the data before real re_tran data */
                handle_re_transimit_frame(f_desc, buf_pos);

                session.state = STATE_TRANSFER;

                break;
            default:
                break;
            }

            break;
        case STATE_TRANSFER:
            //printf("transfering\n");
            if(buf)
            {
                recv_header = (frame_header *)buf;
                wait_seconds = MAX_WAIT_SECONDS;
            }
            else
            {
                if(wait_seconds)
                {
                    wait_seconds--;
                    sleep(1);
                    break;
                }
                else
                {
                    wait_seconds = MAX_WAIT_SECONDS;
                    session.state = STATE_CONN_LOST;
                    break;
                }
             }

            if (recv_header->type != FRAME_TYPE_CONTROL)
            {
                t_log("receive error");
                goto re_login;
            }
            /* handle the sub type */
            switch(recv_header->sub_type)
            {
            case FRAME_CONTROL_CONTINUE:
                printf("transfer re tran\n");
                /* add to transfer list */
                /* skip the data before real re_tran data */
                buf_pos = buf;
                buf_pos += sizeof(frame_header);
                /* file name len:1, file_name:n, file_id:2 */
                buf_pos += 1 + buf_pos[0] + 2;
                
                /* add the re-transmit packages to the list */
                handle_re_transimit_frame(session.f_desc, buf_pos);
               
                break;
             case FRAME_CONTROL_FINISHED:
                printf("send finished\n");
                finish = (finish_frame *)buf;
                if(session.f_desc->file_id == HTONS(finish->file_id))
                {
                    //session.state = STATE_TRANSFER_FIN;
                    t_log("transfer finished\n");
                    //show_mem_stat();
                    session.state = STATE_FILE_INFO_SENT;
                    sleep(2);
                    //pthread_cancel(t_send);
                    //for(t_count = 0; t_count < THREAD_COUNT; t_count++)
                    //    pthread_cancel(t_handle[t_count]);

                    clear_file_desc(session.f_desc);
                }
                else
                    printf("finish frame file id error!\n");
                break;
                
            default:
                break;
            }

            break;
        case STATE_TRANSFER_FIN:
            //for(t_count = 0; t_count < THREAD_COUNT; t_count++)
            //    pthread_cancel(t_handle[t_count]);
            t_log("transfer finished\n");
            session.state = STATE_FILE_INFO_SENT;
//            sleep(1);

            break;
        case STATE_CONN_LOST:
            printf("state connection lost\n");
            //pthread_cancel(t_recv);
            //pthread_cancel(t_send);
            //for(t_count = 0; t_count < THREAD_COUNT; t_count++)
            //    pthread_cancel(t_handle[t_count]);

            
            /* go back to the begining state */
            session.state = STATE_WAIT_CONN;
            sleep(3);
            
            close(session.fd);
            clear_file_desc(session.f_desc);

            //destroy_msg_q(session.package_qid);
            //session.package_qid = -1;

        default:
            break;
        }
        /* free every buf after parse */
        if(buf)
        {
            t_free(buf);
            buf = NULL;
        }
    }

}
