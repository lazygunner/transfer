#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define TRAIN_NO_LEN            20
#define FRAME_DATA_LEN          2048

#define ORIGIN_FRAME            0x00
#define RETRAN_FRAME            0x01


#ifdef VXWORKS
#else
    #define t_lock pthread_mutex_t
    #define t_thread pthread_t
    #define t_sem sem_t
#endif

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
    unsigned char   train_no_len;
    unsigned char   train_no[TRAIN_NO_LEN];
    unsigned short  block_index;
    unsigned short  frame_index;
    unsigned char   data[FRAME_DATA_LEN];

}file_frame_data;

typedef struct file_desc
{
    unsigned char   *file_name;
    FILE            *file_fd;
    int             file_size;      /* max file size < 2GB */
    file_block_desc *block_head;
    file_block_desc *block_tail;
    t_lock          block_list_lock;    
}file_desc;

/* thread pool */
typedef struct t_thread_pool_tag
{
    unsigned char   thread_count;
    unsigned char   *avail_array;
    t_thread        *thread_array;
    t_sem           thread_sem;


}t_thread_pool;

int init_send_file(file_desc *f_desc);
int get_file_info(char *path, unsigned char **buf);
