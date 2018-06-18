#ifndef __RWLOCK_H__
#define __RWLOCK_H__

#include <stdint.h>
#include <pthread.h>

/*
 * Read/write lock definition.
 */
typedef struct {
    pthread_rwlock_t    rwlock; // Used only by posix.c for comparison, not part of my own implementation
    pthread_rwlockattr_t attrs; // Used only by posix.c for comparison, not part of my own implementation

    pthread_mutex_t     lock;               // Make the rwlock a monitor using a mutex
    pthread_cond_t      read_signal;        // Readers wait on this signal when writers are active
    pthread_cond_t      writer_queue;       // Enqueue blocked writers 
    uint32_t            num_readers;        // Number of active readers
    uint32_t            num_writers;        // Number of active writers (should always be 0 or 1)
    uint32_t            blocked_writers;    // Number of writers waiting
} rwlock_t;



void rwlock_init(rwlock_t*);


void rwlock_uninit(rwlock_t*);


void rwlock_lock_rd(rwlock_t*);


void rwlock_unlock_rd(rwlock_t*);


void rwlock_lock_wr(rwlock_t*);


void rwlock_unlock_wr(rwlock_t*);


#endif
