/**
 * utilities_unleashed
 * CS 341 - Fall 2023
 */
 #include"format.h"
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<ctype.h>


void destroy_element(char* destroy) {
    free(destroy);
    destroy = NULL;
}

void destroy_array(char** destroy) {
    char** original = destroy;
    while (*destroy) {
        if (*destroy)
            destroy_element(*destroy);
        destroy++;
    }
    free(original);
}

char** string_split(const char* split, const char* delim) {
    char *s = strdup(split);
    size_t token_a = 1;
    size_t token_u = 0;
    char **tokens = malloc(token_a*sizeof(char*));
    char *token = s;
    char *token_start = s;
    while ((token = strsep(&token_start, delim))) {
        if (token_u == token_a) {
            token_a *= 2;
            tokens = realloc(tokens, token_a * sizeof(char*));
        }
        tokens[token_u++] = strdup(token);
    }
    if (token_u == 0) {
        destroy_element(token);
    } else {
        tokens = realloc(tokens, token_u * sizeof(char*));
    }
    free(s);
    return tokens;
}

int find_cmd(int argc, char* argv[]) {
    int index = -1;
    int i = 0;
    for (; i < argc; i++) {
        if (!strcmp(argv[i], "--")) {
            index = i;
            break;
        }
    }
    return index;
}
int is_arg_valid(int argc, char* argv[]) {
    if (find_cmd(argc, argv) == -1)
        return 0;
    int index = find_cmd(argc, argv);
    if (argc < 3)
        return 0;

    if (argc > 3) {
        int i = 1;
        for (; i< index; i++) {
            if (!strchr(argv[i], '='))
                return 0;
        }
        i = 1;

        for (; i < index; i++) {
            char** check = string_split(argv[i], "=");
            unsigned long j = 0;
            for (; j < strlen(check[0]); j++) {
                if (isdigit(check[0][j]) || isalpha(check[0][j]) || check[0][j] == '_' || check[0][j] == '%')
                    continue;
                else {
                    printf("invalid with %c\n", check[0][j]);
                    free(check);
                    check = NULL;
                    return 0;
                }
            }

            j = 0;
            for (; j < strlen(check[1]); j++) {
                if (isdigit(check[1][j]) || isalpha(check[1][j]) || check[1][j] == '_' || check[1][j] == '%')
                    continue;
                else {
                    printf("invalid with %c\n", check[0][j]);
                    free(check);
                    check = NULL;
                    return 0;
                }
            }
            destroy_array(check);
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (!is_arg_valid(argc, argv)) print_env_usage();
    pid_t child = fork();
    if(child == -1)
        print_fork_failed();
    else if(child == 0){
        int i = 1;
        for(; i< argc-2; i++) {
            if(strcmp(argv[i],"--") == 0)
                break;
            char** splitted = string_split(argv[i], "=");
            char* c = (splitted[1][0] == '%') ? getenv(splitted[1] + 1) : splitted[1];
            if(setenv(splitted[0], c, 1))
                print_environment_change_failed();
            destroy_array(splitted);
        }
        execvp(argv[i+1], &argv[i+1]);
        print_exec_failed();
    } else {
        int status;
        waitpid(child,&status,0);
        return 0;
    }
   return 0;
}