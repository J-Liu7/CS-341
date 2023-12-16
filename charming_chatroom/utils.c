/**
 * charming_chatroom
 * CS 341 - Fall 2023
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    ssize_t fix_size = htonl(size);
    return write_all_to_socket(socket, (char*)&fix_size , MESSAGE_SIZE_DIGITS);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    ssize_t read_ct = 0;
    while (read_ct < (ssize_t) count) {
        ssize_t num_read = read(socket, buffer + read_ct, count - read_ct);
        if (num_read == 0) {
            break;
        } else if (num_read > 0) {
            read_ct += num_read;
        } else if (num_read == -1 && errno != EINTR) {
            return -1;
        }
    }
    return read_ct;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    ssize_t write_ct = 0;
    while (write_ct < (ssize_t) count) {
        ssize_t num_write = write(socket, buffer + write_ct, count - write_ct);
        if (num_write == 0) {
            break;
        } else if (num_write > 0) {
            write_ct += num_write;
        } else if (num_write == -1 && errno != EINTR) {
            return -1;
        }
    }
    return write_ct;
}
