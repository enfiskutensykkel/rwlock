#ifndef __RWLOCK_POSIX_H__
#define __RWLOCK_POSIX_H__

#include "rwlock.h"
#include <pthread.h>

/*
 * Wrapper for pthread_rwlock_t
 * On Linux (using GNU POSIX), rwlock is by default reader-biased. 
 * However, it is possible to set its kind to writer-biased using attributes.
 * On macOS (using clang's POSIX), rwlock is by default writer-biased 
 * but not very well performing.
 */
struct rwlock 
{
    pthread_rwlock_t        rwlock;
    pthread_rwlockattr_t    attrs;
};

#endif
