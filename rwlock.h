#ifndef __RWLOCK_H__
#define __RWLOCK_H__

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>


/*
 * Read/write lock definition.
 */
typedef struct {
    pthread_rwlock_t    rwlock; // Used only by posix.c for comparison, not part of my own implementation

    pthread_spinlock_t  read_lock;  // Ensure exclusive access among readers
    pthread_mutex_t     write_lock; // Ensure exclusive access among writers
    sem_t               read_sem;   // Read-ownership
    sem_t               write_sem;  // Write-ownership
    volatile uint32_t   num_writers;// Number of writers currently waiting for exclusive write-ownership
    volatile uint32_t   num_readers;// Number of concurrent readers
} rwlock_t;



void rwlock_init(rwlock_t*);


void rwlock_uninit(rwlock_t*);


void rwlock_lock_rd(rwlock_t*);


void rwlock_unlock_rd(rwlock_t*);


void rwlock_lock_wr(rwlock_t*);


void rwlock_unlock_wr(rwlock_t*);


#endif
