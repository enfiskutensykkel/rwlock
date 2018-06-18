#ifndef __RWLOCK_H__
#define __RWLOCK_H__

#include <stdint.h>
#include <pthread.h>

/*
 * Read/write lock definition.
 */
typedef struct {
    pthread_spinlock_t  lock;
    uint32_t            num_threads;
    volatile int*       reader_flags;
    volatile int        writer_flag;
} rwlock_t;



void rwlock_init(rwlock_t*, uint32_t num_threads);


void rwlock_uninit(rwlock_t*);


void rwlock_lock_rd(rwlock_t*, uint32_t thread);


void rwlock_unlock_rd(rwlock_t*, uint32_t thread);


void rwlock_lock_wr(rwlock_t*);


void rwlock_unlock_wr(rwlock_t*);


#endif
