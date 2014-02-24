#include "thread.h"
#include "lib/memory.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

void *t_malloc(int x)     
{
    void *ptr;
    ptr = XMALLOC(x, 1);
    //printf("[alloc] ptr:%x size:%d\n", ptr, x);
    return ptr;
}

void t_free(void *x)
{
    //printf("[free]  ptr:%x\n", x);
    return XFREE(x);

}

int init_lock(t_lock *lock)
{
    return pthread_mutex_init(lock, NULL);
}

int lock_t_lock(t_lock *lock)
{
    return pthread_mutex_lock(lock);
}

int unlock_t_lock(t_lock *lock)
{
    return pthread_mutex_unlock(lock);
}

int init_sem(t_sem *sem, unsigned char count)
{
    return sem_init(sem, 0, count);
}

int t_release(t_sem *sem)
{
    return sem_post(sem);
}

int t_aquire(t_sem *sem)
{
    return sem_wait(sem);
}

/* non-block */
int t_aquire_nb(t_sem *sem)
{
    return sem_trywait(sem);
}

int create_thread(t_thread *tid, void *(*func)(void *), void *args)
{
#ifdef VXWORKS
#else
    return pthread_create(tid, NULL, func, args);
#endif
}

int cancel_thread(t_thread *tid)
{
    return pthread_cancel(*tid);
}

int create_msg_q(int key_id)
{
    key_t key;
    int qid;

    if(-1 == (key = ftok(".", key_id)))   // key  
    {  
        perror("Create key error!\n");  
        return NULL;  
    } 

    if(-1 == (qid = msgget(key, IPC_CREAT|IPC_EXCL|0666)))      
    {  
        perror("message queue already exitsted!");  
        return NULL;  
    }  
    return qid;
}

int destroy_msg_q(int qid)
{
    msgctl(qid, IPC_RMID, NULL);
}

int send_to_msg_q(int qid, void *buf, int size, int flag)
{
    return msgsnd(qid, buf, size, flag);
}

int recv_msg_q(int qid, void *buf, int size, int msg_type, int flag)
{
    return msgrcv(qid, buf, size, msg_type, flag);
}
/*
int init_thread_pool(t_thread_pool *t_pool, int count)
{
    int i = 0;
    t_pool = (t_thread_pool)t_malloc(sizeof(t_thread_pool));
    t_pool->thread_count = count;
    t_pool->avail_array = (unsigned char *)t_malloc(count * sizeof(char));
    t_pool->thread_array = (t_thread **)t_malloc(count * sizeof(t_thread *));
    t_pool->thread_sem = (t_sem *)t_malloc(sizeof(t_sem));

    for(i = 0; i < count; i++)
    {   
        t_pool->avail_array[i] = 1; 
        t_pool->thread_array[i] = (t_thread *)malloc(sizeof(t_thread));
    }
    init_sem(t_pool->thread_sem, count);
    
}




*/
