#define _GNU_SOURCE
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include "rwlock.h"


void rwlock_init(rwlock_t* lock)
{
    //pthread_spin_init(&lock->read_lock, PTHREAD_PROCESS_PRIVATE);
    pthread_mutex_init(&lock->read_lock, NULL);
    pthread_mutex_init(&lock->write_lock, NULL);

    sem_init(&lock->read_sem, 0, 1);
    sem_init(&lock->write_sem, 0, 1);

    lock->num_writers = 0;
    lock->num_readers = 0;
}


void rwlock_uninit(rwlock_t* lock)
{
    sem_destroy(&lock->write_sem);
    sem_destroy(&lock->read_sem);
    pthread_mutex_destroy(&lock->write_lock);
    pthread_mutex_destroy(&lock->read_lock);
    //pthread_spin_destroy(&lock->read_lock);
}


void rwlock_lock_rd(rwlock_t* lock)
{
    // Block out writers or wait for writers to release
    // the lock.
    sem_wait(&lock->read_sem);

    // Prevent race-condition with other readers
    //pthread_spin_lock(&lock->read_lock);
    pthread_mutex_lock(&lock->read_lock);

    // If we are the first reader, take write-ownership to 
    // prevent writers from writing while we are reading
    if (lock->num_readers++ == 0) {
        sem_wait(&lock->write_sem);
    }

    //pthread_spin_unlock(&lock->read_lock);
    pthread_mutex_unlock(&lock->read_lock);

    sem_post(&lock->read_sem);
}


void rwlock_unlock_rd(rwlock_t* lock)
{
    // Prevent race condition with other readers
    //pthread_spin_lock(&lock->read_lock);
    pthread_mutex_lock(&lock->read_lock);
    
    lock->num_readers--;

    // If we are the last reader, give back write-ownership so
    // writers can take it.
    if (lock->num_readers == 0) {
        sem_post(&lock->write_sem);
    }

    //pthread_spin_unlock(&lock->read_lock);
    pthread_mutex_unlock(&lock->read_lock);
}


void rwlock_lock_wr(rwlock_t* lock)
{
    // Prevent race condition with other writers
    pthread_mutex_lock(&lock->write_lock);

    // Are we the first writer? If we are, take read-ownership
    // to prevent new readers from reading while we are reading
    if (lock->num_writers++ == 0) {
        sem_wait(&lock->read_sem);
    }

    pthread_mutex_unlock(&lock->write_lock);

    // Take exclusive write access
    sem_wait(&lock->write_sem);

    assert(lock->num_readers == 0);

}


void rwlock_unlock_wr(rwlock_t* lock)
{
    // Release exclusive write access
    sem_post(&lock->write_sem);

    // Prevent race condition with other writers
    pthread_mutex_lock(&lock->write_lock);

    lock->num_writers--;
    
    // If we are the last writer, give back read-ownership
    if (lock->num_writers == 0) {
        sem_post(&lock->read_sem);
    }

    pthread_mutex_unlock(&lock->write_lock);
}

