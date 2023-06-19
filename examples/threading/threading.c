#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
	
	usleep(1000*thread_func_args->wait_to_obtain_ms);	// Sleep for wait_to_obtain_ms
	pthread_mutex_lock(thread_func_args->mutex);			// Lock Mutex
	
	usleep(1000*thread_func_args->wait_to_release_ms);	// Sleep for wait_to_release_ms
	pthread_mutex_unlock(thread_func_args->mutex);		// Unlock Mutex
	
	
    thread_func_args->thread_complete_success = true;	// Sets to true only if thread is completed.
	
	return (void *)thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
	 bool retval;
	 
	 //Allocate memory for the Thread data structure
	 struct thread_data* thread_param = (struct thread_data*)malloc(sizeof(struct thread_data));
	 
	 //Populate the data structure with the requied data
	 thread_param->wait_to_obtain_ms = wait_to_obtain_ms;
	 thread_param->wait_to_release_ms = wait_to_release_ms;
	 thread_param->mutex = mutex;
	 thread_param->thread_complete_success = false;
	 
	retval = pthread_create( thread, // thread id passed as input
										(void*) 0,	// no atributes required
										threadfunc,	// Thread handler
										(void*) thread_param	// arguments for the thread
										);
							
	//pthread_join(*thread, (void *)thread_param);	// Join not required here since we should not block for the thread to complete
	
	if (0 == retval)
		return true;
	else
		return false;
}

