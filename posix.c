#include <pthread.h>
#include "rwlock.h"


void rwlock_init(rwlock_t* lock)
{
    pthread_rwlock_init(&lock->rwlock, NULL);
}


void rwlock_uninit(rwlock_t* lock)
{
    pthread_rwlock_destroy(&lock->rwlock);
}


void rwlock_lock_rd(rwlock_t* lock)
{
    pthread_rwlock_rdlock(&lock->rwlock);
}


void rwlock_unlock_rd(rwlock_t* lock)
{
    pthread_rwlock_unlock(&lock->rwlock);
}


void rwlock_lock_wr(rwlock_t* lock)
{
    pthread_rwlock_wrlock(&lock->rwlock);
}


void rwlock_unlock_wr(rwlock_t* lock)
{
    pthread_rwlock_unlock(&lock->rwlock);
}
