/**
 * critical_concurrency
 * CS 341 - Fall 2023
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    struct queue* to_return = malloc(sizeof(struct queue));
    if (to_return == NULL)
        return NULL;
    
    to_return -> head = NULL;
    to_return -> tail = NULL;
    to_return -> size = 0;
    to_return -> max_size = max_size;
    pthread_cond_init((&to_return -> cv), NULL);
    pthread_mutex_init(&(to_return -> m), NULL);
    return to_return;
}

void queue_destroy(queue *this) {
    /* Your code here */
    if (this == NULL)
        return;
    queue_node* start = this->head;
    while (start) {
        queue_node* p = start;
        start = start -> next;
        free(p);
    }
    pthread_mutex_destroy(&(this->m));
    pthread_cond_destroy(&(this->cv));
    free(this);
}

void queue_push(queue *this, void *data) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));
    while (this->max_size > 0 && this->size == this->max_size)
        pthread_cond_wait(&this->cv, &this->m);
    
    queue_node * add = malloc(sizeof(queue_node));
    add->data = data;
    add->next = NULL;
    if (this->size == 0) {
        this->head = add;
        this->tail = add;
    } else {
        this->tail->next = add;
        this->tail = add;
    }
    this->size++;

    if (this -> size > 0)
        pthread_cond_broadcast(&this->cv);
    
    pthread_mutex_unlock(&this->m);
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));
    while (this->head == NULL)
        pthread_cond_wait(&this->cv, &this->m);

    queue_node * to_remove = this->head;
    void* to_return = to_remove->data;
    if (this->head) {
        this->head = this->head->next;
    } else if (this->size == 0) {
        this->head = NULL;
        this->tail = NULL;
    }
    this->size--;
    if (this->size > 0 && this->size < this->max_size)
        pthread_cond_broadcast(&(this->cv));
    
    pthread_mutex_unlock(&(this->m));
    if (to_remove)
        free(to_remove);
    
    return to_return;
}
