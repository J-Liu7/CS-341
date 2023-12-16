/**
 * password_cracker
 * CS 341 - Fall 2023
 */
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include <string.h>
#include <crypt.h>
#include <stdio.h>
#include "./includes/queue.h"
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

static char* result = NULL;
static int found_pass = 0;
static int number_thread = 0;
static int fin_all = 0;
static int hashes = 0;


static char user_name[10];
static char hash[16];
static char known[16];

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;


void* crack_psswd(void* input) {
    struct crypt_data cdata;
    cdata.initialized = 0;
    size_t index = (size_t) input;
    char* copy_we_know = malloc(16* sizeof(char));

    while (true) {
        
        pthread_barrier_wait(&barrier);
    
        if (fin_all) 
            break;
        

        long start_pos = 0;
        int count = 0;
        long range_ct;
        int output = 0;
        
        
        strcpy(copy_we_know, known);

    
        int prefix_len = getPrefixLength(copy_we_know);
        int unknown_len = strlen(copy_we_know) - prefix_len;
        getSubrange(unknown_len, number_thread, index, &start_pos, &range_ct); 
        setStringPosition(copy_we_know + prefix_len, start_pos);
     
        v2_print_thread_start(index, user_name, start_pos, copy_we_know);
        
     
        long i = 0;
        for(; i < range_ct; i++) {
            char* current_hash = crypt_r(copy_we_know, "xx", &cdata);
            count++;

            if (strcmp(current_hash, hash) == 0) {
                pthread_mutex_lock(&lock);
                result = copy_we_know;
                found_pass = 1;
                v2_print_thread_result(index, count, 0);
                hashes += count; 
                pthread_mutex_unlock(&lock);
                output = 1;
                break;
            }

            pthread_mutex_lock(&lock);
            if (found_pass) {
                v2_print_thread_result(index, count, 1);
                hashes += count;
                pthread_mutex_unlock(&lock);
                output = 1;
                break;
            }
            pthread_mutex_unlock(&lock);
            incrementString(copy_we_know + prefix_len);
        } 
        

        if (!output) {
            pthread_mutex_lock(&lock);
            v2_print_thread_result(index, count, 2);
            hashes += count;
            pthread_mutex_unlock(&lock);
        }

        pthread_barrier_wait(&barrier); 
    }

    free(copy_we_know);
    return NULL;
}


int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads

    pthread_t tids[thread_count];
    pthread_barrier_init(&barrier, NULL, thread_count + 1);
    number_thread = thread_count;

    size_t i = 0;
    for(; i < thread_count; i++) 
        pthread_create(tids + i, NULL, crack_psswd, (void*) i + 1);
    


    size_t length = 0;
    char* buffer = NULL;
    while(getline(&buffer, &length, stdin) != -1) {
        
        if (strlen(buffer) >= 1 && buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = '\0';
      
        sscanf(buffer, "%s %s %s", user_name, hash, known);  
        v2_print_start_user(user_name);

        double begin = getTime();
        double cpu_begin = getCPUTime();

        pthread_barrier_wait(&barrier);
        pthread_barrier_wait(&barrier);
  
        double time = getTime() - begin;
        double cpu_time = getCPUTime() - cpu_begin;
        if (found_pass) {
            v2_print_summary(user_name, result, hashes, time, cpu_time, 0);
        } else {
            v2_print_summary(user_name, result, hashes, time, cpu_time, 1);
        }

        found_pass = 0;
        hashes = 0;
    }   

    fin_all = 1;
    pthread_barrier_wait(&barrier);

    i = 0;
    for (; i < thread_count; i++)
      pthread_join(tids[i], NULL);
    
    if (buffer) 
        free(buffer);
    
    pthread_mutex_destroy(&lock);
    pthread_barrier_destroy(&barrier);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
