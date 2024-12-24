#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
// #define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading DEBUG: \e[1;33m" msg "\e[0m\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: \e[1;31m" msg "\e[0m\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // obtain the thread arguments from the function parameter
    struct thread_data* thread_func_args = (struct thread_data*) thread_param;
    if (NULL == thread_func_args) {
        ERROR_LOG("Invalid thread parameters.");
        return NULL;
    }

    // NOTE: this is not thread-safe
    thread_func_args->pid = pthread_self();
    DEBUG_LOG("Current thread ID: %lu", thread_func_args->pid);

    // Wait before attempting to obtain the mutex
    usleep(thread_func_args->wait_to_obtain_ms * 1000);

    // Attempt to lock the mutex
    if (pthread_mutex_lock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Failed to lock mutex.");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    DEBUG_LOG("Mutex locked by thread %lu", thread_func_args->pid);

    // Hold the mutex for the specified time
    usleep(thread_func_args->wait_to_release_ms * 1000);

    // Unlock the mutex
    if (pthread_mutex_unlock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Failed to unlock mutex.");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    DEBUG_LOG("Mutex unlocked by thread %lu", thread_func_args->pid);

    thread_func_args->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    // Allocate memory for thread_data
    struct thread_data* thread_args = (struct thread_data*) malloc(sizeof(struct thread_data));
    if (NULL == thread_args) {
        ERROR_LOG("Failed to allocate memory for thread data.");
        return false;
    }

    // Initialize thread_data fields (except PID)
    thread_args->mutex = mutex;
    thread_args->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_args->wait_to_release_ms = wait_to_release_ms;
    thread_args->thread_complete_success = false;
    
    // Create the thread
    if (pthread_create(thread, NULL, threadfunc, thread_args) != 0) {
        ERROR_LOG("Failed to create thread.");
        free(thread_args);
        return false;
    }

    DEBUG_LOG("Thread created successfully.");
    return true;
}

