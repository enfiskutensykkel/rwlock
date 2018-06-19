#ifndef __RWLOCK_WP_H__
#define __RWLOCK_WP_H__

#include "rwlock.h"
#include <stdint.h>
#include <pthread.h>


/*
 * R/W lock definition.
 * This lock is writer-biased.
 */
struct rwlock 
{
    pthread_mutex_t     lock;               // Implement r/w lock as an monitor
    pthread_cond_t      reader_signal;      // Readers will wait on this signal when writers are active
    pthread_cond_t      writer_queue;       // Enqueue blocked writers
    uint32_t            num_readers;        // Number of active readers
    uint32_t            num_writers;        // Number of active writers (should always be 0 or 1)
    uint32_t            blocked_writers;    // Number of writers currently trying to take lock
};


#endif
