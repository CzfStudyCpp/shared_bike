#ifndef _DEMO_THREAD_H_INCLUDED_
#define _DEMO_THREAD_H_INCLUDED_

#ifdef __cpplusplus
extern "C" {
#endif // __cpplusplus



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

	typedef intptr_t        int_t;
	typedef uintptr_t       uint_t;
//因为跟log4CPP冲突，增加T前缀
#define  T_OK          0
#define  T_ERROR      -1


	int thread_mutex_create(pthread_mutex_t *mtx);
	int thread_mutex_destroy(pthread_mutex_t *mtx);
	int thread_mutex_lock(pthread_mutex_t *mtx);
	int thread_mutex_unlock(pthread_mutex_t *mtx);


	int thread_cond_create(pthread_cond_t *cond);
	int thread_cond_destroy(pthread_cond_t *cond);
	int thread_cond_signal(pthread_cond_t *cond);
	int thread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx);


#ifdef __cpplusplus
}
#endif // __cpplusplus

#endif /* _DEMO_THREAD_H_INCLUDED_ */




