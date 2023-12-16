/**
 * nonstop_networking
 * CS 341 - Fall 2023
 */
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

char **parse_args(int argc, char **argv);
verb check_args(char **args);

void rd_res(char **args, int sock_res, verb method) {
    char* ok = "OK\n";
    char* err = "ERROR\n";
    char* response = calloc(1,strlen(ok)+1);
    size_t number_reads = read_all_from_socket(sock_res, response, strlen(ok));
    // OK
    if (strcmp(response, ok) == 0) {
        if (method == DELETE || method == PUT) {
            print_success();
        } else if (method == GET) {
            char* localfilename = args[4];
            FILE *loca_fil = fopen(localfilename, "w+");
            if (loca_fil == NULL) {
                perror("fopen");
                exit(-1);
            }
            size_t size;
            read_all_from_socket(sock_res, (char*) &size, sizeof(size_t));
            size_t get_c = 0;
            while (get_c < size + 1024) {
                size_t buff_sz = 0;
                if (size + 1024 - get_c > 1024) {
                    buff_sz = 1024;
                } else {
                    buff_sz = size + 1024 - get_c;
                }
                char buffer[buff_sz];
                size_t number_reads = read_all_from_socket(sock_res, buffer, buff_sz);
                if (number_reads == 0)
                    break;
                fwrite(buffer, 1, number_reads, loca_fil);
                get_c += number_reads;
            }
            if (get_c == 0 && get_c != size) {
                print_connection_closed();
                exit(-1);
            } else if (get_c < size) {
                print_too_little_data();
                exit(-1);
            } else if (get_c > size) {
                print_received_too_much_data();
                exit(-1);
            }
            fclose(loca_fil);
        } else if (method == LIST) {
            size_t size;
            read_all_from_socket(sock_res, (char*)&size, sizeof(size_t));
            char buffer[size + 6];
            memset(buffer, 0, size + 6);
            size_t number_reads = read_all_from_socket(sock_res, buffer, size + 5);
            if (number_reads == 0 && number_reads != size) {
                print_connection_closed();
                exit(-1);
            } else if (number_reads < size) {
                print_too_little_data();
                exit(-1);
            } else if (number_reads > size) {
                print_received_too_much_data();
                exit(-1);
            }
            fprintf(stdout, "%zu%s", size, buffer);
        }
    } else {
        response = realloc(response, strlen(err) + 1);
        read_all_from_socket(sock_res, response + number_reads, strlen(err) - number_reads);
        if (strcmp(response, err) == 0) {
            char err_msg[100];
            size_t num_error_read = read_all_from_socket(sock_res, err_msg, 100);
            if (num_error_read == 0)
                print_connection_closed();
            print_error_message(err_msg);
        } else {
            print_invalid_response();
        }
    }
    free(response);
}

int main(int argc, char **argv) {
    // Good luck!
    char** args = parse_args(argc, argv);
    verb method  = check_args(args);
    char* host = args[0];
    char* port = args[1];
    int sock_res = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_res == -1) {
        perror("Socket Error");
        exit(1);
    }

    struct addrinfo hints;
    struct addrinfo* result; 
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int information_results = getaddrinfo(host, port, &hints, &result);
    if (information_results != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(information_results));
        exit(1);
    }

    int connection_results = connect(sock_res, result->ai_addr, result->ai_addrlen);
    if (connection_results == -1) {
        perror("Connection Error");
        exit(1);
    }

    freeaddrinfo(result);

    char* req_buff;
    if (method == LIST) {
        req_buff = "LIST\n";
    } else {
        char* method_character = args[2];
        char* arg_character = args[3];
        req_buff = malloc(strlen(method_character) + strlen(arg_character) + 3);
        sprintf(req_buff, "%s %s\n", method_character, arg_character);
    }
    size_t nrw = write_all_to_socket(sock_res, req_buff, strlen(req_buff));
    if (nrw < strlen(req_buff)) {
        print_connection_closed();
        exit(-1);
    }
    if (method != LIST)
        free(req_buff);
    
    if (method == GET) {
    } else if (method == PUT) {
        char* filename = args[4];
        struct stat s;
        int file_st = stat(filename, &s);
        if (file_st == -1)
            exit(-1);
        
        size_t file_sz = s.st_size;
        write_all_to_socket(sock_res, ((char*) &file_sz), sizeof(size_t));
        FILE* loca_fil = fopen(filename, "r");
        if (loca_fil == NULL) {
            perror("fopen");
            exit(-1);
        }
        size_t put_count = 0;
        while (put_count < file_sz) {
            size_t buff_sz = 0;
            if (file_sz - put_count > 1024) {
                buff_sz = 1024;
            } else {
                buff_sz = file_sz - put_count;
            }
            char put_buffer[buff_sz];
            fread(put_buffer, 1, buff_sz, loca_fil);
            size_t num_write = write_all_to_socket(sock_res, put_buffer, buff_sz);
            if (num_write < buff_sz) {
                print_connection_closed();
                exit(-1);
            }
            put_count += buff_sz;
        }
        fclose(loca_fil);
    } else if (method == LIST) {

    } else if (method == DELETE) {

    }
    
    int shut_res = shutdown(sock_res, SHUT_WR);
    if (shut_res != 0)
        perror("Shutdown");
    
    rd_res(args, sock_res, method);
    shutdown(sock_res, SHUT_RD);
    close(sock_res);
    free(args);
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
