#include "thread.h"


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

int create_thread(t_thread *tid, void *(*func)(void), void *args)
{
#ifdef VXWORKS
#else
    return pthread_create(tid, NULL, func, args);
#endif
}
