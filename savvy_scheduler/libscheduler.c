#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

/**
 * The struct to hold the information about a given job
 */
typedef struct _information_job {
    int id;

    /* TODO: Add any other information and bookkeeping you need into this
     * struct. */
     double time_arr;
     double time_rem;
     double runtime_recent;
     double time_required;
     double time_start;
     int priority;
} information_job;

int processes;
double time_wait;
double response_time;
double turnaround_time;


void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here
    processes = 0;
    time_wait = 0;
    response_time = 0;
    turnaround_time = 0;
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    // TODO: Implement me!
    information_job *info_a = ((job*)a)->metadata;
    information_job *info_b = ((job*)b)->metadata;
    if (info_a->time_arr < info_b->time_arr) {
        return -1;
    }
    return 1;
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    // TODO: Implement me!
    information_job *info_a = ((job*)a)->metadata;
    information_job *info_b = ((job*)b)->metadata;
    if ((info_a->priority - info_b->priority) == 0) {
        return break_tie(a, b);
    } else if ((info_a->priority - info_b->priority) < 0) {
        return -1;
    }
    return 1;
}

int comparer_psrtf(const void *a, const void *b) {
    // TODO: Implement me!
    information_job *info_a = ((job*)a)->metadata;
    information_job *info_b = ((job*)b)->metadata;
    if ((info_a->time_rem - info_b->time_rem) == 0) {
        return break_tie(a, b);
    } else if ((info_a->time_rem - info_b->time_rem)<0){
        return -1;
    }
    return 1;
}

int comparer_rr(const void *a, const void *b) {
    // TODO: Implement me!
    information_job *info_a = ((job*)a)->metadata;
    information_job *info_b = ((job*)b)->metadata;
    if ((info_a->runtime_recent - info_b->runtime_recent) == 0) {
        return break_tie(a, b);
    } else if ((info_a->runtime_recent - info_b->runtime_recent) < 0) {
        return -1;
    }
    return 1;
}

int comparer_sjf(const void *a, const void *b) {
    // TODO: Implement me!
    information_job *info_a = ((job*)a)->metadata;
    information_job *info_b = ((job*)b)->metadata;
    if ((info_a->time_required - info_b->time_required) == 0) {
        return break_tie(a, b);
    } else if ((info_a->time_required - info_b->time_required) < 0) {
        return -1;
    }
    return 1;
}

void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO: Implement me!
    information_job *info = malloc(sizeof(information_job));
    info->id = job_number;
    info->time_start = -1;
    info->priority = sched_data->priority;
    info->time_arr = time;
    info->time_rem = sched_data->running_time;
    info->runtime_recent = -1;
    info->time_required = sched_data->running_time;
    newjob->metadata = info;
    priqueue_offer(&pqueue, newjob);
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    if (!job_evicted) {
        return priqueue_peek(&pqueue);
    }
    // set time for current job
    information_job* info = job_evicted->metadata;
    info->runtime_recent = time;
    info->time_rem -= 1;
    if (info->time_start < 0) {
        info->time_start = time -1;
    }
    if (pqueue_scheme == PPRI || pqueue_scheme == PSRTF || pqueue_scheme == RR) {
        job* current = priqueue_poll(&pqueue);
        priqueue_offer(&pqueue, current);
        return priqueue_peek(&pqueue);
    }
    return job_evicted;
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO: Implement me!
    processes += 1;
    information_job* j = job_done->metadata;
    time_wait += time - j->time_arr - j->time_required;
    response_time += j->time_start - j->time_arr;
    turnaround_time += time - j->time_arr;
    free(j);
    priqueue_poll(&pqueue);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO: Implement me!
    return time_wait/processes;
}

double scheduler_average_turnaround_time() {
    // TODO: Implement me!
    return turnaround_time/processes;
}

double scheduler_average_response_time() {
    // TODO: Implement me!
    return response_time/processes;
}

void scheduler_show_queue() {

}


void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}