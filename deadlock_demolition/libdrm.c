/**
 * deadlock_demolition
 * CS 341 - Fall 2023
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct drm_t {pthread_mutex_t mutex;};
set* seetproo = NULL;
static graph* g = NULL;

drm_t *drm_init() {
    /* Your code here */
    pthread_mutex_lock(&mutex);
    if (g == NULL)
        g = shallow_graph_create();
    
    drm_t* to_return = malloc(sizeof(drm_t));
    pthread_mutex_init(&(to_return->mutex), NULL);
    graph_add_vertex(g, to_return);
    pthread_mutex_unlock(&mutex);
    return to_return;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&mutex);
    int to_return = 0;
    if (!graph_contains_vertex(g, thread_id)) {
        goto out;
    } else {
        if (graph_adjacent(g, drm, thread_id)) {
            to_return = 1;
            graph_remove_edge(g, drm, thread_id);
            pthread_mutex_unlock(&drm->mutex);
        }
    }
    out:
    pthread_mutex_unlock(&mutex);
    return to_return;
}

int cir_exist(void* input) {
    if (seetproo == NULL)
        seetproo = shallow_set_create();
    
    if (set_contains(seetproo, input)) {
        seetproo = NULL;
        return 1;
    } else {
        set_add(seetproo, input);
        vector* neighbor = graph_neighbors(g, input);
        size_t i = 0;
        for (; i < vector_size(neighbor); i++) {
            if (cir_exist(vector_get(neighbor, i)))
                return 1;
        }
        seetproo = NULL;
        return 0;
    }
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&mutex);
    graph_add_vertex(g, thread_id);
    if (!graph_adjacent(g, drm, thread_id)) {
        graph_add_edge(g, thread_id, drm);
        if (cir_exist(thread_id) == 0) {
            pthread_mutex_unlock(&mutex);
            pthread_mutex_lock(&drm->mutex);
            pthread_mutex_lock(&mutex);
            graph_remove_edge(g, thread_id, drm);
            graph_add_edge(g, drm, thread_id);
            pthread_mutex_unlock(&mutex);
            return 1;
        } else {
            graph_remove_edge(g, thread_id, drm);
            pthread_mutex_unlock(&mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

void drm_destroy(drm_t *drm) {
    graph_remove_vertex(g, drm);
    pthread_mutex_destroy(&drm->mutex);
    free(drm);
    return;
}

