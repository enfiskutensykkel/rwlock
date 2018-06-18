#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include "rwlock.h"


enum reader_states
{
    READER_INACTIVE = 0,        // Reader not interested in read-lock
    READER_WAITING  = 1,        // Reader waiting for read-lock
    READER_TAKING   = 2,        // Reader is trying to take read-lock
    READER_ACTIVE   = 3         // Reader thinks he has read-lock
};

enum writer_states
{
    WRITE_LOCK_UNLOCKED = 0,    // Write-lock is not used
    WRITE_LOCK_LOCKED   = 1     // Write-lock is used
};


void rwlock_init(rwlock_t* lock, uint32_t num_threads)
{
    pthread_spin_init(&lock->lock, PTHREAD_PROCESS_PRIVATE);
    lock->num_threads = num_threads;
    lock->reader_flags = calloc(num_threads, sizeof(int));
    
    for (uint32_t i = 0; i < num_threads; ++i) {
        lock->reader_flags[i] = READER_INACTIVE;
    }

    lock->writer_flag = WRITE_LOCK_UNLOCKED;
}


void rwlock_uninit(rwlock_t* lock)
{
    pthread_spin_destroy(&lock->lock);
    free((void*) lock->reader_flags);
}


void rwlock_lock_rd(rwlock_t* lock, uint32_t thread)
{
    // Indicate that we are attempting to take the lock
    lock->reader_flags[thread] = READER_TAKING;

    pthread_spin_lock(&lock->lock);
    if (lock->writer_flag == WRITE_LOCK_LOCKED) {
        // Microoptimization as starting a while loop has a bigger overhead
        // than a simple if-check
        while (lock->writer_flag == WRITE_LOCK_LOCKED) {

            // Write-lock was taken, we need to yield
            pthread_spin_unlock(&lock->lock); // Lock first, allowing for others to take the lock
            lock->reader_flags[thread] = READER_WAITING;

            // Microoptimization for test-test and set, there is no need trying again
            // if we haven't seen a change in the flag
            while (lock->writer_flag == WRITE_LOCK_LOCKED) {
                pthread_yield();
            }

            // Indicate that we are trying to take lock again
            lock->reader_flags[thread] = READER_TAKING;
            pthread_spin_lock(&lock->lock);
        }
    }
    pthread_spin_unlock(&lock->lock);

    // We got the lock
    lock->reader_flags[thread] = READER_ACTIVE;
}


void rwlock_lock_wr(rwlock_t* lock)
{
    // Try to take write-lock, if not already taken
    pthread_spin_lock(&lock->lock);
    while (lock->writer_flag == WRITE_LOCK_LOCKED) {
        pthread_spin_unlock(&lock->lock);

        pthread_yield();

        pthread_spin_lock(&lock->lock);
    }
    
    // Take the write-lock
    lock->writer_flag = WRITE_LOCK_LOCKED;
    pthread_spin_unlock(&lock->lock);

    // Now we need to wait for existing readers to complete
    while (1) {
        uint32_t i;

        // Check if any readers think they have the lock or is about to take the lock
        for (i = 0; i < lock->num_threads; ++i) {
            if (lock->reader_flags[i] == READER_TAKING || lock->reader_flags[i] == READER_ACTIVE) {
                break;
            }
        }

        // We made it all the way here without finding any active readers,
        // or readers trying to take the lock. Lets have a go at it.
        if (i == lock->num_threads) {
            break;
        }
        else {
            pthread_yield();
        }
    }
}


void rwlock_unlock_rd(rwlock_t* lock, uint32_t thread)
{
    // Indicate that we are no longer interested in the read-lock
    lock->reader_flags[thread] = READER_INACTIVE;
}


void rwlock_unlock_wr(rwlock_t* lock)
{
    lock->writer_flag = WRITE_LOCK_UNLOCKED;
}
