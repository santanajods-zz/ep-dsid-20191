#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "threadpool.h"

typedef struct work_struct{
	void (*routine) (void*);
	void * arg;
	struct work_struct* next;
} work_t;

typedef struct _threadpool_struct {
	int num_threads;	//number of threads
	int queue_size;			//number of threads in the queue
	pthread_t *threads;	//pointer to threads
	work_t* queue_head;		//queue head pointer
	work_t* queue_tail;		//queue tail pointer
	pthread_mutex_t queue_lock;		//lock on the queue list
	pthread_cond_t queue_not_empty;	//non empty and empty condidtion vairiables
	pthread_cond_t queue_empty;
	int shutdown;
	int dont_accept;
} _threadpool;

void* doWork(threadpool p) {
	//printf("entrou\n");
	_threadpool * pool = (_threadpool *) p;
	work_t* cur;	//The q element
	int k;
	while(1) {
		pool->queue_size = pool->queue_size;
		pthread_mutex_lock(&(pool->queue_lock));  //get the q lock.
		//printf("listening\n");

		while( pool->queue_size == 0) {	//if the size is 0 then wait.  
			if(pool->shutdown) {
				pthread_mutex_unlock(&(pool->queue_lock));
				pthread_exit(NULL);
			}
			//wait until the condition says its no emtpy and give up the lock. 
			pthread_mutex_unlock(&(pool->queue_lock));  //get the queue_lock.
			if(pool->num_threads <= 50) printf("Thread %lu waiting...\n", pthread_self());
			pthread_cond_wait(&(pool->queue_not_empty),&(pool->queue_lock));

			//check to see if in shutdown mode.
			if(pool->shutdown) {
				pthread_mutex_unlock(&(pool->queue_lock));
				pthread_exit(NULL);
			}
		}

		cur = pool->queue_head;	//set the cur variable.  

		pool->queue_size--;		//decriment the size.  

		if(pool->queue_size == 0) {
			pool->queue_head = NULL;
			pool->queue_tail = NULL;
		}
		else {
			pool->queue_head = cur->next;
		}

		if(pool->queue_size == 0 && ! pool->shutdown) {
			//the q is empty again, now signal that its empty.
			pthread_cond_signal(&(pool->queue_empty));
		}
		pthread_mutex_unlock(&(pool->queue_lock));
		(cur->routine) (cur->arg);  //actually do work.
		free(cur);	//free the work storage.  	
	}
}

threadpool create_threadpool(int num_threads_in_pool) {
  _threadpool *pool;
	int i;

  if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
    return NULL;

  pool = (_threadpool *) malloc(sizeof(_threadpool));
  if (pool == NULL) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;
  }

  pool->threads = (pthread_t*) malloc (sizeof(pthread_t) * num_threads_in_pool);

  if(!pool->threads) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;	
  }

  pool->num_threads = num_threads_in_pool; //set up structure members
  pool->queue_size = 0;
  pool->queue_head = NULL;
  pool->queue_tail = NULL;
  pool->shutdown = 0;
  pool->dont_accept = 0;

  //initialize mutex and condition variables.  
  if(pthread_mutex_init(&pool->queue_lock,NULL)) {
    fprintf(stderr, "Mutex initiation error!\n");
	return NULL;
  }
  if(pthread_cond_init(&(pool->queue_empty),NULL)) {
    fprintf(stderr, "CV initiation error!\n");	
	return NULL;
  }
  if(pthread_cond_init(&(pool->queue_not_empty),NULL)) {
    fprintf(stderr, "CV initiation error!\n");	
	return NULL;
  }

  //make threads

  for (i = 0;i < num_threads_in_pool;i++) {
	  if(pthread_create(&(pool->threads[i]),NULL,doWork,pool)) {
	    fprintf(stderr, "Thread initiation error!\n");	
		return NULL;		
	  }
  }
  return (threadpool) pool;
}


void dispatch(threadpool from_me, dispatch_fn dispatchDestiny,
	      void *arg) {
  _threadpool *pool = (_threadpool *) from_me;
	work_t * cur;
	int k;

	k = pool->queue_size;

	//make a work queue element.  
	cur = (work_t*) malloc(sizeof(work_t));
	if(cur == NULL) {
		fprintf(stderr, "Out of memory creating a work struct!\n");
		return;	
	}

	cur->routine = dispatchDestiny;
	cur->arg = arg;
	cur->next = NULL;

	pthread_mutex_lock(&(pool->queue_lock));

	if(pool->dont_accept) { //Just incase someone is trying to queue more
		free(cur); //work structs.  
		return;
	}
	if(pool->queue_size == 0) {
		pool->queue_head = cur;  //set to only one
		pool->queue_tail = cur;
		pthread_cond_signal(&(pool->queue_not_empty));  //I am not empty.  
	} else {
		pool->queue_tail->next = cur;	//add to end;
		pool->queue_tail = cur;			
	}
	pool->queue_size++;
	pthread_mutex_unlock(&(pool->queue_lock));  //unlock the queue.
}

void killThreadpool(threadpool destroyme) {
	_threadpool *pool = (_threadpool *) destroyme;
	void* nothing;
	int i = 0;
	free(pool->threads);

	pthread_mutex_destroy(&(pool->queue_lock));
	pthread_cond_destroy(&(pool->queue_empty));
	pthread_cond_destroy(&(pool->queue_not_empty));
	return;
}

