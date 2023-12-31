/**
 * teaching_threads
 * CS 341 - Fall 2023
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */

/* You should create a start routine for your threads. */

typedef struct thread {
    int * list;
    size_t len;
    reducer reduce_func;
    int base_case;
} thread;

void* start_routine(void* pointer_) {
    thread * pointer__ = (thread*) pointer_;
    int *result = malloc(sizeof(int));
    result = &(pointer__->base_case);
    size_t i = 0;
    for (; i < pointer__->len; i++) 
        *result = pointer__->reduce_func(*result, pointer__->list[i]);
    return (void*) result;
}

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if (list_len <= num_threads)
        return reduce(list, list_len, reduce_func, base_case);
    
    pthread_t tid[num_threads];
    int processing_list[num_threads];
    thread* thread_data[num_threads];
    int num_splited = list_len / num_threads;
    size_t i = 0;
    for (; i< num_threads; i++) {
        thread_data[i] = malloc(sizeof(thread));
        thread_data[i]->list = list + i * num_splited;
        thread_data[i]->reduce_func = reduce_func;
        thread_data[i]-> base_case = base_case;
        if (i == num_threads - 1) {
            thread_data[i]-> len = list_len - i * num_splited;
        } else {
            thread_data[i]->len = num_splited;
        }
    }
    i = 0;
    for (; i < num_threads; i++)
        pthread_create(&tid[i], 0, start_routine, (void*) thread_data[i]);

    i = 0;
    for (; i < num_threads; i++) {
        void* res;
        pthread_join(tid[i], &res);
        processing_list[i] = *((int*) res);
    }  
    return reduce(processing_list, num_threads, reduce_func, base_case);
}
