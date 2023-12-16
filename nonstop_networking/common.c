/**
 * nonstop_networking
 * CS 341 - Fall 2023
 */
#include "common.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    size_t wrd_ct = 0;
    while (wrd_ct < count) {
        ssize_t result = read(socket, buffer + wrd_ct, count - wrd_ct);
        if (result == 0)
            break;
        
        if (result == -1 && errno == EINTR) {
            continue;
        } else if (result == -1) {
            perror("read from socket");
            return -1;
        }
        wrd_ct += result;
    }
    return wrd_ct;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    size_t wrd_ct = 0;
    while (wrd_ct < count) {
        ssize_t result = write(socket, buffer + wrd_ct, count - wrd_ct);
        if (result == 0)
            break;
        
        if (result == -1 && errno == EINTR) {
            continue;
        } else if (result == -1) {
            perror("write to socket");
            return -1;
        }
        wrd_ct += result;
    }
    return wrd_ct;
}


