#include <tuple>
#include <unistd.h>
#include <iostream>
#include "threadpool.h"

pthread_mutex_t ThreadPool::mutexQueue;
sem_t ThreadPool::sem;
std::queue<Task> ThreadPool::workToDo;

int ThreadPool::enqueue(Task work) {
	// Acquire the mutex lock
	pthread_mutex_lock(&mutexQueue);

	// Push 'work' into the queue
	workToDo.push(work);

	// Release the mutex lock
	pthread_mutex_unlock(&mutexQueue);

  return 0;
}

Task ThreadPool::dequeue() {
	// Acquire the mutex lock
	pthread_mutex_lock(&mutexQueue);

	// Retrieve the work in the head of queue and assign to 'work'
	Task work = workToDo.front();
	// Remove it from the queue
	workToDo.pop();

	// Release the mutex lock
	pthread_mutex_unlock(&mutexQueue);

	return work;
}

void *ThreadPool::worker(void *param) {
	ThreadPool *self = reinterpret_cast<ThreadPool *>(param);
	Task work;

  while(!workToDo.empty()) {
		// Acquire the semaphore
		sem_wait(&sem);

		// Remove the work from the queue
	  work = self->dequeue();

		// Release the semaphore
		sem_post(&sem);

		// Run the specified function
	  self->execute(work.function, work.data);
	}

  pthread_exit(NULL);
}

void ThreadPool::execute(void (*somefunction)(void *p), void *p) {
  somefunction(p);
}

ThreadPool::ThreadPool() {
	// Create a mutex
	pthread_mutex_init(&mutexQueue,NULL);

	// Create a semaphore
	sem_init(&sem, 0, 1);

	// Create threads
  for(int i = 0; i < NUMBER_OF_THREADS; i++) {
    pthread_create(&thread[i], NULL, ThreadPool::worker, this);
	}
}

int ThreadPool::submit(void (*somefunction)(void *), void *p) {
	// Push tasks to the queue by calling enqueue() function
	Task work;
	work.function = somefunction;
	work.data = p;
  enqueue(work);

  return 0;
}

void ThreadPool::shutdown() {
  for (int i = 0; i < NUMBER_OF_THREADS; i++) {
    pthread_join(thread[i], NULL);
	}

	// Destroy the mutex and semaphore once all task have completed
  pthread_mutex_destroy(&mutexQueue);
	sem_destroy(&sem);
}
