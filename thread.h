#ifndef _THREAD_H_
#define _THREAD_H_


#include <pthread.h>
#include <semaphore.h>

#ifdef VXWORKS
#else
    #define t_thread pthread_t
    #define t_lock pthread_mutex_t
    #define t_sem sem_t
#endif

/* thread pool */
typedef struct t_thread_pool_tag
{
    unsigned char   thread_count;
    unsigned char   *avail_array;
    t_thread        **thread_array;
    t_sem           *thread_sem;

}t_thread_pool;

#endif
