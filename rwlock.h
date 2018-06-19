#ifndef __RWLOCK_H__
#define __RWLOCK_H__

#include <stdint.h>


typedef struct rwlock rwlock_t;


void rwlock_init(rwlock_t* lock, uint32_t num_threads);


void rwlock_uninit(rwlock_t* lock);


void rwlock_lock_rd(rwlock_t* lock, uint32_t thread);


void rwlock_unlock_rd(rwlock_t* lock, uint32_t thread);


void rwlock_lock_wr(rwlock_t* lock);


void rwlock_unlock_wr(rwlock_t* lock);

#endif
