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



#endif
