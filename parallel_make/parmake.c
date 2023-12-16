#include <stdbool.h> 
#include <pthread.h>
#include <stdio.h>
#include "parmake.h"
#include "parser.h"
#include <time.h>
#include "format.h"
#include "graph.h"
#include <unistd.h>
#include "set.h"
#include <sys/stat.h>


graph * G;
vector * goal_vec;
pthread_mutex_t innn;
pthread_mutex_t outtt;
pthread_cond_t cv;

int dispatch_task(char * rule);
rule_t * rule_next(rule_t * rule);
int rule_execute(rule_t * rule);

bool file_check(char * file_name) {
	FILE * file;
	if ((file = fopen(file_name, "r"))) {
		fclose(file);
		return true;
	} else {
		return false;
	}
}

bool cycle_detector(char * node, set * visited) {
	if (set_contains(visited, node)) {
		return true;
	} else {
		set_add(visited, node);
		bool result = false;
		size_t i;
		vector * neigh_vec = graph_neighbors(G, node);
		size_t neigh_size = vector_size(neigh_vec);
		char * current_rule = NULL;
		for (i = 0; i < neigh_size; i++) {
			current_rule = vector_get(neigh_vec, i);
			result = result || cycle_detector(current_rule, visited);
			if (result) {
				vector_destroy(neigh_vec);
				return true;
			}
			set_remove(visited, current_rule);
		}
		vector_destroy(neigh_vec);
		return false;
	}
}

void organize(set * illegal, set * legal) {	
	
	vector * grv = graph_neighbors(G, "");
	
	size_t i;
	bool cycle = false;
	char * cgr = NULL;
	size_t grs = vector_size(grv);
	set * visited = NULL;
	for (i = 0; i < grs; i++) {
		visited = string_set_create();
		cgr = vector_get(grv, i);
		cycle = cycle_detector(cgr, visited);
		if (cycle) {
			set_add(illegal, cgr);
		} else {
			set_add(legal, cgr);
		}
		set_destroy(visited);
		visited = NULL;
	}
	vector_destroy(grv);
	return;
}

rule_t * rule_next(rule_t * current_rule) {

	pthread_mutex_lock(&innn);
	int crs = current_rule->state;
	pthread_mutex_unlock(&innn);
	if (crs != 0)
		return NULL;
	

	rule_t * next = NULL;
	vector * child = graph_neighbors(G, current_rule->target);
	size_t c_size = vector_size(child);

	if (c_size == 0) {
		next = current_rule;
	} else {
		size_t i;
		bool all_done = true;
		bool failed = false;
		rule_t * kid_rul = NULL;
		int kid_stat = 0;
		for (i = 0; i < c_size; i++) {
			kid_rul = graph_get_vertex_value(G, vector_get(child, i));
			pthread_mutex_lock(&innn);
			kid_stat = kid_rul->state;
			pthread_mutex_unlock(&innn);
			if (kid_stat == 0) {
				next = rule_next(kid_rul);
				all_done = false;
				if (next != 0)
					break;
				
			} else if (kid_stat == 1) {
				all_done = false;
			} else if (kid_stat == -1) {
				failed = true;
			}
		 		
		}
		if (all_done) {
			if (failed == true) {
				current_rule->state = -1;
			} else {
				next = current_rule;
			}
		}
	}
	vector_destroy(child);
	return next;
}

rule_t * rule_fetch() {
	rule_t * current_goal = NULL;
	rule_t * rule = NULL;
	size_t i;
	for (i = 0; i < vector_size(goal_vec); i++) {
		current_goal = graph_get_vertex_value(G, vector_get(goal_vec, i));
		pthread_mutex_lock(&innn);
		if (current_goal->state) {
			pthread_mutex_unlock(&innn);
			vector_erase(goal_vec, i);
			if (vector_size(goal_vec) == 0) {
				pthread_cond_broadcast(&cv);
				pthread_mutex_unlock(&outtt);
				pthread_exit(0);
			}	
			return rule_fetch();
		}
		pthread_mutex_unlock(&innn);
		rule = rule_next(current_goal);
		if (rule != NULL) {
			rule->state = 1;
			break;
		}

	}
	return rule;
}

void * worker_make(void * num_threads) {
	while (true) {	
		pthread_mutex_lock(&outtt);
		if (vector_size(goal_vec) == 0) {
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&outtt);
			pthread_exit(0);
		}
		rule_t * current_rule = NULL;
		while (true) {
			current_rule = rule_fetch();
			if (current_rule != NULL)
				break;
			
			if (vector_size(goal_vec) == 0) {
				pthread_cond_broadcast(&cv);
				pthread_mutex_unlock(&outtt);
				pthread_exit(0);
			}
			if ((size_t) num_threads != 1)
				pthread_cond_wait(&cv, &outtt);
			
		}
		pthread_mutex_unlock(&outtt);
		rule_execute(current_rule);
		pthread_cond_broadcast(&cv);
	}
} 


int rule_execute(rule_t * current_rule) {
	char * rule = current_rule->target;
	bool isfile = file_check(rule);
	bool is_new = false;
	size_t i;
	if (isfile) {
		vector * child = graph_neighbors(G, rule);
		size_t c_size = vector_size(child);
		char * kid_rul = NULL;
		for (i = 0; i < c_size; i++) {
			kid_rul = vector_get(child, i);
			if (file_check(kid_rul)) {
				struct stat child_stat;
				struct stat parent_stat;
				stat(kid_rul, &child_stat);
				stat(rule, &parent_stat);
				if (difftime(parent_stat.st_mtime, child_stat.st_mtime) > 0) {
					continue;
				} else {
					is_new = true;
					break;
				}
			}
		}
		vector_destroy(child);
	}
	int retval = 2;
	if (isfile == false || is_new == true) {
		char * current_command = NULL;
		vector * cmd_vector = current_rule->commands;
		size_t cmd_size = vector_size(cmd_vector);
		for (i = 0; i < cmd_size; i++) {
			current_command = vector_get(cmd_vector, i);
			if (system(current_command)) {
				retval = -1;
				break;
			}
		}
	}
	pthread_mutex_lock(&innn);
	current_rule->state = retval;
	pthread_mutex_unlock(&innn);
	return retval;
}


int parmake(char *makefile, size_t num_threads, char **targets) {
	
	G = parser_parse_makefile(makefile, targets);

	set * illegal = string_set_create();
	set * legal = string_set_create();	
	organize(illegal, legal);
	
	SET_FOR_EACH(illegal, current_rule, {print_cycle_failure(current_rule);});

	goal_vec = string_vector_create();
	SET_FOR_EACH(legal, current_rule, {vector_push_back(goal_vec, current_rule);});
	pthread_cond_init(&cv, NULL);
	pthread_mutex_init(&outtt, NULL);
	pthread_mutex_init(&innn, NULL);

	size_t i;
	pthread_t threads[num_threads];
	for (i = 0; i < num_threads; i++) {
		pthread_create(&threads[i], NULL, worker_make, (void*) num_threads);
	}

	for (i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);
	
	set_destroy(illegal);
	set_destroy(legal);
	vector_destroy(goal_vec);
	graph_destroy(G);
	pthread_cond_destroy(&cv);
	pthread_mutex_destroy(&outtt);
	pthread_mutex_destroy(&innn);
	return 0;
}