/**
 * vector
 * CS 341 - Fall 2023
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    char* c;
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    sstring* pointer = malloc(sizeof(sstring));
    pointer -> c = strdup(input);
    return pointer;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    assert(input);
    return strdup(input -> c);
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    if (this && addition) {
        this -> c = realloc(this -> c, strlen(this -> c) + strlen(addition -> c) + 1);
        strcat(this -> c, addition -> c);
        return (strlen(this -> c));
    } else if (this) 
        return strlen(this -> c);
      else if (addition)
        return strlen(addition -> c);
    return -1;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    assert(this);
    vector* vec = string_vector_create();
    char* interator = this -> c;
    char* start = this -> c;
    while(*interator) {
        if (*interator == delimiter) {
            *interator = '\0';
            vector_push_back(vec, start);
            start = interator + 1;
            *interator = delimiter;
        }
        interator++;
    }
    vector_push_back(vec, start);
    return vec;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    assert(this);
    char* first = strstr(this -> c + offset, target);
    if (first) {
        char* temp = malloc(strlen(this -> c) + strlen(substitution) - strlen(target) + 1);
        int before = first - (this -> c);
        char* sub = temp + before;
        char* remain_temp = sub + strlen(substitution);
        char* remain = first + strlen(target);
        strncpy(temp, this -> c, before);
        strcpy(sub, substitution);
        strcpy(remain_temp, remain);
        free(this -> c);
        this -> c = temp;
        return 0;
    }
    return -1;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    char* slice = malloc(end - start + 1);
    strncpy(slice, this -> c + start, end - start);
    slice[end - start] = '\0';
    return slice;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    assert(this);
    if (this -> c) {
        free(this -> c);
        this -> c = NULL;
    }
    free(this);
    this = NULL;
}
