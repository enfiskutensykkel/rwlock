#define _GNU_SOURCE
#include <pthread.h>
#include "rwlock.h"
#include "posix.h"


void rwlock_init(rwlock_t* lock, uint32_t num_threads)
{
#ifndef __APPLE__
#warning "Writer-preferring GNU POSIX rwlocks may not be portable and work as intended"
    pthread_rwlockattr_init(&lock->attrs);

    // Use PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP for optimised for writers.
    // Use PTHREAD_RWLOCK_PREFER_READER_NP (or attrs=NULL) for optimised for writers.
    // Preferring readers is the default behaviour.
    pthread_rwlockattr_setkind_np(&lock->attrs, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);

    pthread_rwlock_init(&lock->rwlock, &lock->attrs);
#else
    // On macOS, pthread_rwlock_t is writer-preferring by default
    pthread_rwlock_init(&lock->rwlock, NULL);
#endif
}


void rwlock_uninit(rwlock_t* lock)
{
    pthread_rwlock_destroy(&lock->rwlock);
#ifndef __APPLE__
    pthread_rwlockattr_destroy(&lock->attrs);
#endif
}


void rwlock_lock_rd(rwlock_t* lock, uint32_t thread)
{
    pthread_rwlock_rdlock(&lock->rwlock);
}


void rwlock_unlock_rd(rwlock_t* lock, uint32_t thread)
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
