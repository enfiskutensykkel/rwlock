#include <pthread.h>
#include <assert.h>
#include "rwlock.h"
#include "wrpref.h"


void rwlock_init(rwlock_t* lock, uint32_t num_readers)
{
    pthread_mutex_init(&lock->lock, NULL);
    pthread_cond_init(&lock->writer_queue, NULL);
    pthread_cond_init(&lock->reader_signal, NULL);
    lock->num_writers = 0;
    lock->num_readers = 0;
    lock->blocked_writers = 0;
}


void rwlock_uninit(rwlock_t* lock)
{
    assert(lock->num_readers == 0);
    assert(lock->num_writers == 0);
    assert(lock->blocked_writers == 0);

    pthread_mutex_destroy(&lock->lock);
    pthread_cond_destroy(&lock->writer_queue);
    pthread_cond_destroy(&lock->reader_signal);
}


void rwlock_lock_rd(rwlock_t* lock, uint32_t thread)
{
    pthread_mutex_lock(&lock->lock);

    // As writing is infrequent, always prioritize writers.
    while (lock->blocked_writers > 0 || lock->num_writers > 0) {
        pthread_cond_wait(&lock->reader_signal, &lock->lock);
    }

    lock->num_readers++;

    pthread_mutex_unlock(&lock->lock);

    assert(lock->num_writers == 0);
}


void rwlock_lock_wr(rwlock_t* lock)
{
    pthread_mutex_lock(&lock->lock);

    // There can be no readers and only one writer at the time (when writing).
    // Wait for others to complete
    while (lock->num_readers > 0 || lock->num_writers > 0) {

        // Indicate that we are blocked from writing
        lock->blocked_writers++;

        pthread_cond_wait(&lock->writer_queue, &lock->lock);

        // We are not blocked at the moment
        lock->blocked_writers--;
    }

    // Now we are an active writer
    lock->num_writers++;

    pthread_mutex_unlock(&lock->lock);

    assert(lock->num_readers == 0);
    assert(lock->num_writers == 1);
}


void rwlock_unlock_rd(rwlock_t* lock, uint32_t thread)
{
    pthread_mutex_lock(&lock->lock);

    lock->num_readers--;

    // If we are the last reader, wake up a single blocked writer.
    // There is no need to wake up more than one, as there can only be 
    // one writer at the time
    if (lock->num_readers == 0 && lock->blocked_writers > 0) {
        pthread_cond_signal(&lock->writer_queue);
    }

    pthread_mutex_unlock(&lock->lock);
}


void rwlock_unlock_wr(rwlock_t* lock)
{
    pthread_mutex_lock(&lock->lock);

    lock->num_writers--;
    assert(lock->num_writers == 0);

    // If there are any blocked writers, wake one up.
    // Otherwise, wake up all readers.
    if (lock->blocked_writers > 0) {
        pthread_cond_signal(&lock->writer_queue);
    }
    else {
        pthread_cond_broadcast(&lock->reader_signal);
    }

    pthread_mutex_unlock(&lock->lock);
}

