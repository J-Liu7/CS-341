/**
 * password_cracker
 * CS 341 - Fall 2023
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include <crypt.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "./includes/queue.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static queue* tasks;
static int recovered_number;
static size_t number_tasks;


void* crack_psswd(void* thread_num) {
    size_t index = (size_t) thread_num;
    char username[10];
    char hash[16];
    char known[16];
    struct crypt_data cdata;
    cdata.initialized = 0;
    char* task = NULL;
    while (true) {
        task = queue_pull(tasks);
        if (!task)
            break;
        
        sscanf(task, "%s %s %s", username, hash, known);
    
        v1_print_thread_start(index, username);
        int prefix_lnth = getPrefixLength(known);
     
        setStringPosition(prefix_lnth + known, 0);
        int hashes = 0;
        double start_time = getThreadCPUTime();
        char* curr_hast = NULL;
        int fail = 1;
 
        while (1) {
            curr_hast = crypt_r(known, "xx", &cdata);
            hashes++;
       
            if (strcmp(curr_hast, hash) == 0) {
                pthread_mutex_lock(&lock);
                recovered_number++;
                pthread_mutex_unlock(&lock);
                fail = 0;
                break;
            }
         
            int result = incrementString(prefix_lnth + known);
            if (result == 0)
                break;
            
        }
        double time = getThreadCPUTime() - start_time;
        v1_print_thread_result(index, username, known, hashes, time, fail);
        free(task);
        task = NULL;
    }
    return NULL;
}


int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    tasks = queue_create(100);
    pthread_t tids[thread_count];
    
    size_t length = 0;
    char* buffer = NULL;
    while(getline(&buffer, &length, stdin) != -1) {
        
        if (strlen(buffer) >= 1 && buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = '\0';
           
        queue_push(tasks, strdup(buffer));
        number_tasks++;
    }

    size_t i = 0;
    for(; i < thread_count; i++)
        queue_push(tasks, NULL);
    

    
    i = 0;
    for(; i < thread_count; i++)
        pthread_create(tids + i, NULL, crack_psswd, (void*) i + 1);
    

    
    i = 0;
    for(; i < thread_count; i++)
        pthread_join(tids[i], NULL);
    

    v1_print_summary(recovered_number, number_tasks - recovered_number);

    
    free(buffer);
    buffer = NULL;
    queue_destroy(tasks);
    pthread_mutex_destroy(&lock);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
