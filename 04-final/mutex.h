/*
 * mutex.h
 *
 *
 *  Created on: Feburary 20 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *
 */


#ifndef MUTEX
#define MUTEX

#include "pcb.h"
#include "que.h"

typedef struct mutex_type {
	int mutex_state;  /* 0: unlocked / Available for use
	 	 	 	 	 	 1: locked / Currently in use
	 	 	 	 	  */

	//Thread/Something that currently holds the mutex

} mutex;
typedef mutex * mutex_ptr;

mutex_ptr mutex_constructor();

/* Locks the given mutex. If the mutex is currently unlocked, it becomes
 * locked by the calling thread, and mutex_lock returns
 * immediately. If the mutex is already locked by another thread/something, mutex_lock
 * suspends the calling thread (puts it in the blocked state) until the mutex is unlocked.
 * mutex_lock will return a 1 if the lock
 */
int mutex_lock (mutex_ptr this);

/*
 * Acts identically to mutex_lock, except that it does not block the calling thread if the
 * mutex is already locked by another thread. Instead, mutex_trylock returns immediately indicating whether
 * the mutex in locked or not.
 * RETURN INFO HERE
 */
int mutex_trylock (mutex_ptr this);

/*
 * Unlocks the given mutex. The mutex is assumed to be locked and owned by the calling thread when mutex_unlock is called.
 * RETURN INFO HERE
 */
int mutex_unlock (mutex_ptr this);

typedef struct cond_type {
	que_ptr waiting_threads;
	que_ptr associated_mutex;
} cond;
typedef cond * cond_ptr;

cond_ptr cond_constructor();

/*
 * Block on a condition variable and add thread to waiting que (waiting for signal)
 */
int cond_wait(cond_ptr this, mutex_ptr this2);

/*
 * Unblocks at least one of the threads that are blocked on the specified
 * condition variable this (if any threads are blocked on this).
 */
int cond_signal(cond_ptr this);


#endif /* MUTEX */

