#ifndef __RWLOCK_OPTIMIZED_H__
#define __RWLOCK_OPTIMIZED_H__

#include "rwlock.h"
#include <stdint.h>
#include <pthread.h>

/*
 * Read/write lock definition.
 * This lock is writer-biased.
 */
struct rwlock 
{
    uint32_t            num_threads;    // Total number of threads (both readers and writers) taking the lock
    volatile int*       reader_states;  // One state per reader
    pthread_mutex_t     lock;           // Accesses to writer flag should be atomic
    volatile int        writer_lock;    // Writer lock
};

#endif
