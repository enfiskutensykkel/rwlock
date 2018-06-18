#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include "rwlock.h"


struct stats
{
    pthread_t       thread;
    uint64_t        min;
    uint64_t        max;
    double          acc;
    size_t          held;
};


struct data
{
    rwlock_t        lock;
    uint64_t        target;
    uint64_t        value;
    uint64_t        read_sleep;
    uint64_t        write_sleep;
    uint32_t        num_threads;
    struct stats    stats[];
};


static void show_statistics(const struct stats* stats, size_t num_writers, size_t num_readers)
{
    uint32_t worker;

    fprintf(stdout, "===== Writers =====\n");
    for (worker = 0; worker < num_writers; ++worker) {
        fprintf(stdout, "[%03u] held=%lu, min=%lu, max=%lu, avg=%.3f\n",
                worker, stats[worker].held, stats[worker].min, stats[worker].max, stats[worker].acc / stats[worker].held);
    }

    fprintf(stdout, "===== Readers =====\n");
    for (; worker < num_readers + num_writers; ++worker) {
        fprintf(stdout, "[%03u] held=%lu, min=%lu, max=%lu, avg=%.3f\n",
                worker, stats[worker].held, stats[worker].min, stats[worker].max, stats[worker].acc / stats[worker].held);
    }
}


static void pretend_to_work(uint64_t nanoseconds)
{
    struct timespec ts;
    ts.tv_sec = nanoseconds / ((time_t) 1e9);
    ts.tv_nsec = nanoseconds - ts.tv_sec;

    nanosleep(&ts, NULL);
}


static uint64_t get_ns()
{
    struct timespec ts;

    uint64_t ns = 0;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        return ULONG_MAX;
    }

    ns = ts.tv_sec * ((uint64_t) 1e9);
    ns += ts.tv_nsec;
    return ns;
}


static uint32_t get_thread_num(const struct data* data)
{
    for (uint32_t i = 0; i < data->num_threads; ++i) {
        if (data->stats[i].thread == pthread_self()) {
            return i;
        }
    }

    assert(false);
    return 0;
}


/*
 * Reader thread.
 * Try to take the read lock as many times as possible.
 */
static void reader(struct data* data)
{
    uint32_t self = get_thread_num(data);
    struct stats* stats = &data->stats[self];

    stats->min = ULONG_MAX; // Minimum delay waiting for lock
    stats->max = 0;         // Maximum delay waiting for lock
    stats->acc = 0;
    stats->held = 0;        // Number of times the read-lock was held


    while (1) {
        uint64_t before = get_ns();
        rwlock_lock_rd(&data->lock, self);
        uint64_t after = get_ns();

        uint64_t elapsed = after - before;

        uint64_t value = data->value;
//        fprintf(stderr, "[%03u] taking read lock (value=%lu, readers=%u, waiting writers=%u, waited=%lu)\n", 
//                self, value, data->lock.num_readers, data->lock.blocked_writers, elapsed);
        fprintf(stderr, "[%03u] taking read lock (waited=%lu)\n", self, elapsed);

        // Are we there yet?
        if (value == data->target) {
//            fprintf(stderr, "[%03u] releasing read lock (readers=%u)\n", 
//                    self, data->lock.num_readers);
            fprintf(stderr, "[%03u] releasing read lock\n", self);
            rwlock_unlock_rd(&data->lock, self);
            break;
        }

#ifndef NDEBUG
        // Give away control in order to provoke bad behaviour
        // and then do some sanity checking incase our lock is 
        // not functionally correct.
        pthread_yield();
        assert(value == data->value);
#endif

        // Update statistics
        stats->min = (elapsed < stats->min) ? elapsed : stats->min;
        stats->max = (elapsed > stats->max) ? elapsed : stats->max;
        stats->acc += elapsed;
        stats->held++;

        // Holding the read-lock only affects writers
//        fprintf(stderr, "[%03u] releasing read lock (readers=%u)\n", 
//                self, data->lock.num_readers);
        fprintf(stderr, "[%03u] releasing read lock\n", self);

        rwlock_unlock_rd(&data->lock, self);

        pretend_to_work(data->read_sleep);
    }

    pthread_exit(NULL);
}



/*
 * Do some infrequent updating of the shared data.
 */
static void writer(struct data* data)
{
    uint32_t self = get_thread_num(data);
    struct stats* stats = &data->stats[self];

    stats->min = ULONG_MAX; // Minimum delay waiting for lock
    stats->max = 0;         // Maximum delay waiting for lock
    stats->held = 0;        // Number of times the write-lock was held
    
    while (1) {
        uint64_t before = get_ns();
        rwlock_lock_wr(&data->lock);
        uint64_t after = get_ns();

        // Do some sanity checking
        assert(data->lock.num_readers == 0);

        uint64_t value = data->value;

        uint64_t elapsed = after - before;
//        fprintf(stderr, "[%03u] taking write lock (value=%lu, readers=%u, waiting writers=%u, waited=%lu)\n", 
//                self, value, data->lock.num_readers, data->lock.blocked_writers, elapsed);
        fprintf(stderr, "[%03u] taking write lock (waited=%lu)\n", self, elapsed);

        // Are we there yet?
        if (value == data->target) {
            fprintf(stderr, "[%03u] releasing write lock\n", self);
            rwlock_unlock_wr(&data->lock);
            break;
        }

#ifndef NDEBUG
        // Give away control in order to provoke bad behaviour
        // and then do some sanity checking incase our lock is 
        // not functionally correct
        pthread_yield();
        assert(value == data->value);
        assert(data->lock.num_readers == 0);
#endif

        // Update value
        data->value++;

        // Holding the write-lock is expensive (affects both readers 
        // and writers), so we should release it as soon as possible.
        fprintf(stderr, "[%03u] releasing write lock\n", self);

        rwlock_unlock_wr(&data->lock);

        // Update statistics
        stats->min = (elapsed < stats->min) ? elapsed : stats->min;
        stats->max = (elapsed > stats->max) ? elapsed : stats->max;
        stats->acc += elapsed;
        stats->held++;

        pretend_to_work(data->write_sleep);
    }

    pthread_exit(NULL);
}


int main()
{
    size_t num_workers = NUM_READERS + NUM_WRITERS;

    struct data* data = malloc(sizeof(struct data) + sizeof(struct stats) * num_workers);
    if (data == NULL) {
        fprintf(stderr, "Failed to allocate worker data: %s\n", strerror(errno));
        abort();
    }

    rwlock_init(&data->lock, num_workers);
    data->target = NUM_UPDATES;
    data->value = 0;
    data->read_sleep = SLEEP_READ;
    data->write_sleep = SLEEP_WRITE;
    data->num_threads = num_workers;

    // Start worker threads
    uint32_t i;
    for (i = 0; i < NUM_WRITERS; ++i) {
        int err = pthread_create(&data->stats[i].thread, NULL, (void *(*)(void*)) writer, data);
        if (err != 0) {
            fprintf(stderr, "Failed to start worker thread: %s\n", strerror(err));
            abort();
        }
    }
    for (; i < num_workers; ++i) {
        int err = pthread_create(&data->stats[i].thread, NULL, (void *(*)(void*)) reader, data);
        if (err != 0) {
            fprintf(stderr, "Failed to start worker thread: %s\n", strerror(err));
            abort();
        }
    }

    // Wait for all threads to finish
    for (size_t i = 0; i < num_workers; ++i) {
        pthread_join(data->stats[i].thread, NULL);
    }

    show_statistics(data->stats, NUM_WRITERS, NUM_READERS);

    rwlock_uninit(&data->lock);
    free(data);

    return 0;
}

