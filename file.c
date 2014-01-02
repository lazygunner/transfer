#include "file.h"
#include "error.h"
#include <dirent.h>
#include <sys/stat.h>

#define FILE_BLOCK_SIZE     1024 * 1024     /* 1MB */
#define FILE_FRAME_SIZE     1024 * 2        /* 2KB */

#define FILE_INFO_LEN       9                   /* 4 + 4 + 1 */     

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

int read_thread()
{
    



}

int get_file_info(char *path, unsigned char **buf)
{
    struct dirent *ent;
    struct stat file_stat;
    unsigned short name_len = 0;
    unsigned short total_len = 0;
    unsigned int mod_time = 0;
    unsigned int file_size = 0;
    unsigned int offset = 0;
    unsigned char file_count = 0;
    DIR *dir;
    FILE *fp;
    char path_buf[256];
    char *full_path;
    
    full_path = &path_buf[0];
    dir=opendir(path);
    while (NULL != (ent=readdir(dir)))
    {
        if(strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0)
            continue;
        memset(full_path, 0, 256);
        strcpy(full_path + strlen(strcpy(full_path, path)), ent->d_name);
        if(fp = fopen(full_path, "r"))
        {
            name_len = strlen(ent->d_name);
            total_len += FILE_INFO_LEN + name_len;
            file_count++;
            fclose(fp);
        }
    }
    close(dir);
    
    //n files and count(n)
    if (NULL == (*buf = (unsigned char*)t_malloc(total_len + 1)))
        return -1;
    
    dir=opendir(path);
    /* 1 for file count */
    *buf[0] = file_count;
    offset++;

    while (NULL != (ent=readdir(dir)))
    {
        if(strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0)
            continue;
        name_len = strlen(ent->d_name);
        memset(full_path, 0, 256);

        strcpy(full_path + strlen(strcpy(full_path, path)), ent->d_name);
        /*     mod time = st_mtime
            access time = st_atime*/
       
       if(stat(full_path, &file_stat) != -1)
        {
            mod_time = (unsigned int)file_stat.st_mtime;
            file_size = (unsigned int)file_stat.st_size;
            memcpy(*buf + offset, &file_size, 4);
            offset += 4;
            memcpy(*buf + offset, &file_stat, 4);
            offset += 4;
            memcpy(*buf + offset, (char *)&name_len, 1);
            offset += 1;
            memcpy(*buf + offset, ent->d_name, name_len);
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

void handle_thread(void *args)
{   
    transfer_session    *session;
    file_desc           *f_desc;
    file_block_desc     *b_desc;
    
    session = (transfer_session *)args;
    f_desc = session->f_desc;

    for(;;)
    {
        b_desc = get_first_block(f_desc);
        if(b_desc == ERR_BLOCK_LIST_NULL)
            return;
        else if(b_desc == ERR_BLOCK_LIST_EMPTY)
        {
            session->state = STATE_TRANSFER_FIN;
            return; 
        }
        
        printf("block:%d\n", b_desc->index->block_index);
            
    }


}





