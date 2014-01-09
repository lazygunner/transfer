#include "file.h"
#include "error.h"
#include <dirent.h>
#include <sys/stat.h>

#define FILE_BLOCK_SIZE     1024 * 1024     /* 1MB */
#define FILE_FRAME_SIZE     1024 * 2        /* 2KB */
#define MAX_FRAME_COUNT     512

#define FILE_INFO_LEN       9                   /* 4 + 4 + 1 */     
#define THREAD_COUNT 3

#define DEBUG 1
static int get_file_size(FILE *fp)
{
    int size = 0;

    fseek(fp, 0, SEEK_END);      // seek to end of file
    size = ftell(fp);  // get current file pointer
    fseek(fp, 0, SEEK_SET);   // seek back to beginning of file
    return size;
}

static int init_send_list(file_desc *f_desc)
{
    int block_count = 0;
    file_block_desc *block_desc = NULL;
    frame_index *f_index = NULL;

    if(NULL == (f_desc->block_head =\
        (file_block_desc *)t_malloc(sizeof(file_block_desc))))
        return ERR_MALLOC;
    f_desc->block_tail = f_desc->block_head;

    while(block_count * FILE_BLOCK_SIZE < f_desc->file_size)
    {
        if(NULL == (block_desc =\
            (file_block_desc *)t_malloc(sizeof(file_block_desc))))
        {
            t_free(f_desc->block_head);
            return ERR_MALLOC;
        }
        
        if(NULL == (f_index = (frame_index *)t_malloc(sizeof(f_index))))
        {
            t_free(f_desc->block_head);
            t_free(block_desc);
            return ERR_MALLOC;
        }
        
        /* init current block index, all block frame index is 0xFF*/
        f_index->block_index = block_count + 1;
        f_index->frame_index = 0xFF;
        f_index->next = NULL;

        block_desc->index = f_index;
        block_desc->retry_flag = ORIGIN_FRAME;
        block_desc->next = NULL;

        /* append blocks to the list tail */
        f_desc->block_tail->next = block_desc;
        f_desc->block_tail = block_desc;

        block_count++;
    }
    
    return RET_SUCCESS;

}

int init_send_file(file_desc *f_desc)
{

    if(NULL == f_desc->file_name)
        return ERR_FILE_NAME_NULL;
    if(NULL == (f_desc->file_fd = fopen(f_desc->file_name, "rb")))
        return ERR_FILE_NOT_EXIST;
    if(0 == (f_desc->file_size = get_file_size(f_desc->file_fd)))
        return ERR_FILE_SIZE_ZERO;
    
    t_lock *block_list_lock = t_malloc(sizeof(t_lock));
    init_lock(block_list_lock);
    f_desc->block_list_lock = block_list_lock;

    return init_send_list(f_desc);
    
}

int get_file_info(char *path, unsigned char **buf)
{
    DIR *dir = NULL;
    FILE *fp = NULL;
    struct dirent *ent = NULL;
    struct stat file_stat;
    unsigned short name_len = 0;
    unsigned short total_len = 0;
    unsigned int mod_time = 0;
    unsigned int file_size = 0;
    unsigned int offset = 0;
    unsigned char file_count = 0;
    char path_buf[128];
    
    dir = opendir(path);
    while (NULL != (ent=readdir(dir)))
    {
        if(strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0)
            continue;
        memset(path_buf, 0, 128);
        strcpy(path_buf + strlen(strcpy(path_buf, path)), ent->d_name);
        if(fp = fopen(path_buf, "r"))
        {
            name_len = strlen(ent->d_name);
            total_len += FILE_INFO_LEN + name_len;
            file_count++;
            fclose(fp);
        }
    }
    close(dir);
    
    //n files and count(n)
    if ((*buf = (unsigned char *)t_malloc(total_len + 1)) == NULL)
        return -1;
    
    dir=opendir(path);
    /* 1 for file count */
    (*buf)[0] = file_count;
    offset++;

    while (NULL != (ent=readdir(dir)))
    {
        if(strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0)
            continue;
        name_len = strlen(ent->d_name);
        memset(path_buf, 0, 128);

        strcpy(path_buf + strlen(strcpy(path_buf, path)), ent->d_name);
        /*     mod time = st_mtime
            access time = st_atime*/
       
       if(stat(path_buf, &file_stat) != -1)
        {
            mod_time = (unsigned int)file_stat.st_mtime;
            file_size = (unsigned int)file_stat.st_size;
            memcpy((*buf) + offset, &file_size, 4);
            offset += 4;
            memcpy((*buf) + offset, &file_stat, 4);
            offset += 4;
            memcpy((*buf) + offset, (char *)&name_len, 1);
            offset += 1;
            memcpy((*buf) + offset, ent->d_name, name_len);
            offset += name_len;
        }
    } 
    close(dir);
    
    return offset;
}

static file_block_desc *get_first_block(file_desc *f_desc)
{
    file_block_desc *block;

    if(!f_desc || !f_desc->block_head)
        return ERR_BLOCK_LIST_NULL;
    if(f_desc->block_head == f_desc->block_tail)
        return ERR_BLOCK_LIST_EMPTY;
    
    block = f_desc->block_head->next;

    lock_t_lock(f_desc->block_list_lock);

    if(block == f_desc->block_tail)
        f_desc->block_tail = f_desc->block_head;
    f_desc->block_head->next = block->next;

    unlock_t_lock(f_desc->block_list_lock);
    
    return block;
}

void read_thread(void *args)
{
    transfer_session    *session;
    file_desc           *f_desc;
    file_block_desc     *b_desc;
    file_frame_data     *file_frame;
    frame_index         *f_index;
    FILE                *fp;
    q_msg               f_msg;
    int                 err;

    char test[10];

    int i = 0;
    unsigned int read_count = 0;
    
    session = (transfer_session *)args;
    f_desc = session->f_desc;

    if(NULL == (fp = fopen(f_desc->file_name, "r")))
    {
        t_log("open file error");
        return;
    }
   
    for(;;)
    {
        b_desc = get_first_block(f_desc);
        if(b_desc == ERR_BLOCK_LIST_NULL)
        {
            sleep(10);
            continue;
        }
        else if(b_desc == ERR_BLOCK_LIST_EMPTY)
        {
            //session->state = STATE_TRANSFER_FIN;
            sleep(10);
            continue;
        }
        else
        {
            if(b_desc->retry_flag == ORIGIN_FRAME)
            {
                f_index = b_desc->index;
                for(i = 0; i < MAX_FRAME_COUNT; i++)
                {
                    file_frame = (file_frame_data *)t_malloc(sizeof(file_frame_data));
                    file_frame->file_id = f_desc->file_id;
                    file_frame->block_index = f_index->block_index;
                    file_frame->frame_index = i + 1;
                    
                    if(fseek(fp, (f_index->block_index - 1) * FILE_BLOCK_SIZE +\
                        i * FILE_FRAME_SIZE, SEEK_SET) < 0)
                    {
                        printf("seek out of range");
                        t_free(file_frame);
                        return;
                    }
                    read_count = fread(file_frame->data, FILE_FRAME_SIZE, 1, fp);
                    if(read_count < 0)
                        memset(file_frame->data + read_count, 0xFF, FILE_FRAME_SIZE - read_count);
                    
                    
                    f_msg.msg_type = MSG_TYPE_FILE_FRAME;/* must be > 0 */
                    memcpy(f_msg.msg_buf, &file_frame, sizeof(file_frame));
                    while((err = send_to_msg_q(f_desc->qid, &f_msg, sizeof(q_msg), 0)) < 0)  
                    {  
                            perror("msg closed! quit the system!");
                            printf("%d %d\n", f_desc->qid, f_msg.msg_type);
                    } 
#if 0       
                    memset(test, 0, 10);
                    memcpy(test, file_frame->data, 4);
                    memcpy(test + 5, file_frame->data + 2044, 4);
                    test[4] = ' ';
                    printf("%s\n", test);
                  
#endif                    
                }
            }

        }

        
//        printf("block:%d\n", b_desc->index->block_index);
            
    }



}

void send_thread(void *args)
{
    transfer_session    *session;
    file_desc           *f_desc;
    file_block_desc     *b_desc;
    file_frame_data     *file_frame;
    frame_index         *f_index;
    frame_header        *f_header;
    int                 qid;
    q_msg               f_msg;
    int                 err;
    int                 waiting_count = 10;
    int                 frame_len = 0, len = 0;

    char                buf_send[2061];
#if DEBUG
    char test[10];
    FILE *fp;
    fp = fopen("receive.txt", "w");
#endif

    
    session = (transfer_session *)args;
    f_desc = session->f_desc;


    if((qid = f_desc->qid) < 0)
    {
        printf("msg q error");
        return;
    }

    frame_header_init(FRAME_TYPE_DATA, FRAME_DATA_MONITOR, &f_header);
    for(;;)
    {
            /* get frame from msg q */
            if ((err = recv_msg_q(qid, &f_msg, sizeof(q_msg),\
                    MSG_TYPE_FILE_FRAME, IPC_NOWAIT)) < 0)  
            {  
                    perror("recv msg error!");
                    if(waiting_count == 0)
                    {
                    #if DEBUG
                        fclose(fp);
                    #endif
                        t_free(f_header);
                        session->state = STATE_TRANSFER_FIN;
                        return;
                    }
                    else
                        waiting_count--;
                    sleep(1);
                    continue;
            }
            waiting_count = 10;
            file_frame = *((file_frame_data **)(f_msg.msg_buf));
            /* encapusulate frame */
            frame_len = frame_build(f_header, file_frame, sizeof(file_frame_data), buf_send);
            /* send file info frame */
            len = send_file_data(session, buf_send, frame_len);
            
            
#if DEBUG
            memset(test, 0, 10);
            memcpy(test, file_frame->data, 4);
            memcpy(test + 5, file_frame->data + 2044, 4);
            test[4] = ' ';
            printf("%s\n", test);
            fseek(fp, (file_frame->block_index - 1) * FILE_BLOCK_SIZE +\
                    (file_frame->frame_index - 1) * FILE_FRAME_SIZE, SEEK_SET);
            fwrite(file_frame->data, 2048, 1, fp);
#endif
            /* free the data after send */
            t_free(file_frame);

    }

}


void handle_thread(void *args)
{   
    transfer_session    *session;
    file_desc           *f_desc;
    file_block_desc     *b_desc;
    
    session = (transfer_session *)args;
    f_desc = session->f_desc;

}





