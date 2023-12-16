/**
 * nonstop_networking
 * CS 341 - Fall 2023
 */
#include <stdio.h>
#include "format.h"
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include "./includes/dictionary.h"
#include "./includes/vector.h"
#include "common.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>


static char* dir;
static vector* files;
static dictionary* fd_to_connection_state;
static dictionary* server_file_size;
static int epoll_fd;

typedef struct client_info {
    /**
     * state = 0: ready to parse header 
     * state = 1: ready to execute command
     * state = -1: bad request
     * state = -2: bad file size
     * state = -3: no such file
     */ 
	int state;
	verb command;
    char header[1024];
	char filename[255];
} client_info;

void sp_handle() {}

void close_cli(client_fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
    free(dictionary_get(fd_to_connection_state, &client_fd));
    dictionary_remove(fd_to_connection_state, &client_fd);
}

void exit_ser() {
    close(epoll_fd);
    vector_destroy(files);
	vector *infos = dictionary_values(fd_to_connection_state);
	VECTOR_FOR_EACH(infos, info, {
    	free(info);
	});
	vector_destroy(infos);
	dictionary_destroy(fd_to_connection_state);
    dictionary_destroy(server_file_size);
    DIR* d = opendir(dir);
    if (d != NULL) {
        struct dirent* pointer;
        while ((pointer = readdir(d))) {
          if (!strcmp(pointer -> d_name, ".") || !strcmp(pointer -> d_name, ".."))
            continue;
          
          char path[strlen(dir) + strlen(pointer -> d_name) + 1];
          sprintf(path, "%s/%s", dir, pointer -> d_name);
          int result = unlink(path);
          if (result != 0)
              perror("remove file");
          
      }
      closedir(d);
    } else {
        puts("fail");
    }
    rmdir(dir);
	exit(1);
}

int proc_put(int client_fd) {
    client_info* info = dictionary_get(fd_to_connection_state, &client_fd);
    char path[strlen(dir) + strlen(info -> filename) + 2];
    memset(path, 0, strlen(dir) + strlen(info -> filename) + 2);
    sprintf(path, "%s/%s", dir, info -> filename);
    FILE* fp_check = fopen(path, "r");
    if (fp_check == NULL) {
        vector_push_back(files, info->filename);
    } else {
        fclose(fp_check);
    }
    FILE* fp = fopen(path, "w");
    if (fp == NULL)  {
        perror("fopen in put");
        return 1;
    }
    size_t fil_siz;
    read_all_from_socket(client_fd, (char*) &fil_siz, sizeof(size_t));
    size_t rd_cnt = 0;
    while (rd_cnt < fil_siz + 1024) {
        size_t buffer_size = 0;
        if (fil_siz + 1024 - rd_cnt > 1024) {
            buffer_size = 1024;
        } else {
            buffer_size = fil_siz + 1024 - rd_cnt;
        }
        char buffer[buffer_size];
        ssize_t number_read = read_all_from_socket(client_fd, buffer, buffer_size);
        if (number_read == 0) {
            break;
        }
        fwrite(buffer, 1, number_read, fp);
        rd_cnt += number_read;
    }
    fclose(fp);
    if (rd_cnt != fil_siz) {
        remove(path);
        return 1;
    }
    dictionary_set(server_file_size, info -> filename, &fil_siz);
    return 0;
}

void proc_get(int client_fd) {
    client_info* info = dictionary_get(fd_to_connection_state, &client_fd);
    char path[strlen(dir) + strlen(info -> filename) + 2];
    memset(path, 0, strlen(dir) + strlen(info -> filename) + 2);
    sprintf(path, "%s/%s", dir, info -> filename);
    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        info -> state = -3;
        return;
    }
    char* ok = "OK\n";
    write_all_to_socket(client_fd, ok, strlen(ok));
    size_t fil_siz = *((size_t*) dictionary_get(server_file_size, info -> filename));
    write_all_to_socket(client_fd, (char*) &fil_siz, sizeof(size_t));
    size_t get_cnt = 0;
    while (get_cnt < fil_siz) {
        size_t buffer_size = 0;
        if (fil_siz - get_cnt > 1024) {
            buffer_size = 1024;
        } else {
            buffer_size = fil_siz - get_cnt;
        }
        char buffer[buffer_size];
        size_t number_read = fread(buffer, 1, buffer_size, fp);
        if (number_read == 0) {
            break;
        }
        write_all_to_socket(client_fd, buffer, number_read);
        get_cnt += buffer_size;
    }
    fclose(fp);
    close_cli(client_fd);
}

void proc_delete(int client_fd) {
    client_info* info = dictionary_get(fd_to_connection_state, &client_fd);
    char path[strlen(dir) + strlen(info -> filename) + 2];
    memset(path, 0, strlen(dir) + strlen(info -> filename) + 2);
    sprintf(path, "%s/%s", dir, info -> filename);
    if (remove(path) == -1) {
        info -> state = -3;
        return;
    }
    dictionary_remove(server_file_size, info -> filename);
    size_t i = 0;
    for (; i < vector_size(files); i++) {
        char* file = vector_get(files, i);
        if (strcmp(info -> filename, file) == 0) {
            break;
        }
    }
    if (i >= vector_size(files)) {
        info -> state = -3;
        return;
    }
    vector_erase(files, i);
    char* ok = "OK\n";
	write_all_to_socket(client_fd, ok, strlen(ok));
    close_cli(client_fd);
}

void proc_list(int client_fd) {
    char* ok = "OK\n";
	write_all_to_socket(client_fd, ok, strlen(ok));
    if (vector_size(files) == 0) {
        size_t responese_size = 0;
        write_all_to_socket(client_fd, (char*) &responese_size, sizeof(size_t));
        close_cli(client_fd);
    }
    size_t responese_size = 0;
    size_t i = 0;
    for (; i < vector_size(files); i++) {
        char* curr_filename = vector_get(files, i);
        responese_size += strlen(curr_filename) + 1;
    }
    if (responese_size >= 1) {
        responese_size--;
    }
    write_all_to_socket(client_fd, (char*) &responese_size, sizeof(size_t));
    i = 0;
    for (; i < vector_size(files); i++) {
        char* curr_filename = vector_get(files, i);
        write_all_to_socket(client_fd, curr_filename, strlen(curr_filename));
        if (i != vector_size(files) - 1) {
            write_all_to_socket(client_fd, "\n", 1);
        }
    }
    close_cli(client_fd);
}

void parse_head(int client_fd) {
    client_info* info = dictionary_get(fd_to_connection_state, &client_fd);
    size_t rd_cnt = 0;
    int bad_request = 0;
    while (rd_cnt < 1024) {
        if (info -> header[strlen(info -> header) - 1] == '\n') {
            break;
        }
        ssize_t result = read(client_fd, info -> header + rd_cnt, 1);
        if (result == 0) {
            break;
        }
        if (result == -1 && errno == EINTR) {
            continue;
        } else if (result == -1) {
            perror("read from client_fd");
            exit(1);
        }
        rd_cnt += result;
    }
    if (strncmp(info->header, "PUT", 3) == 0) {
		info -> command = PUT;
		strcpy(info->filename, info->header + strlen("PUT") + 1);
		info -> filename[strlen(info->filename)-1] = '\0';
        if (proc_put(client_fd) == 1) {
            info -> state = -2;
            struct epoll_event ev_client;
            memset(&ev_client, '\0', sizeof(struct epoll_event));
            ev_client.events = EPOLLOUT;
            ev_client.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev_client);
            return;
        }
	} else if (strncmp(info->header, "GET", 3) == 0) {
		info -> command = GET;
		strcpy(info->filename, info->header + strlen("GET") + 1);
		info -> filename[strlen(info->filename) - 1] = '\0';
	} else if (strncmp(info->header, "DELETE", 6) == 0) {
		info -> command = DELETE;
		strcpy(info->filename, info->header + strlen("DELETE") + 1);
		info->filename[strlen(info->filename) - 1] = '\0';
	} else if (!strncmp(info->header, "LIST", 4)) {
        if (rd_cnt != 5) {
            bad_request = 1;
        }
		info -> command = LIST;
	} else {
		info -> state = -1;
		struct epoll_event ev_client;
        memset(&ev_client, '\0', sizeof(struct epoll_event));
        ev_client.events = EPOLLOUT;
        ev_client.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev_client);
        return;
	}
    if (bad_request == 1) {
        info -> state = -1;
		struct epoll_event ev_client;
        memset(&ev_client, '\0', sizeof(struct epoll_event));
        ev_client.events = EPOLLOUT;
        ev_client.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev_client);
        return;
    }
	info -> state = 1;
	struct epoll_event ev_client;
    memset(&ev_client, '\0', sizeof(struct epoll_event));
    ev_client.events = EPOLLOUT;
    ev_client.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev_client);
}

void proc_client(int client_fd) {
    client_info* info = dictionary_get(fd_to_connection_state, &client_fd);
    int client_state = info -> state;
    if (client_state == 0) {
        parse_head(client_fd);
    } else if (client_state == 1) {
        char* ok = "OK\n";
        if (info -> command == PUT) {
            write_all_to_socket(client_fd, ok, strlen(ok));
            close_cli(client_fd);
        } else if (info -> command == GET) {
            proc_get(client_fd);
        } else if (info -> command == DELETE) {
            proc_delete(client_fd);
        } else if (info -> command == LIST) {
            proc_list(client_fd);
        }
    } else { 
        char* err = "ERROR\n";
        write_all_to_socket(client_fd, err, strlen(err));
        if (client_state == -1) {
                 
            write_all_to_socket(client_fd, err_bad_request, strlen(err_bad_request));
        } else if (client_state == -2) {
            
            write_all_to_socket(client_fd, err_bad_file_size, strlen(err_bad_file_size));
        } else if (client_state == -3) {
            
            write_all_to_socket(client_fd, err_no_such_file, strlen(err_no_such_file));
        }
        close_cli(client_fd);
    }
}

void run_server(char* port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    } 

    int opt_val = 1;
    int ret_val_1 = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
    if (ret_val_1 == -1) {
        perror("setsockopt");
        exit(1);
    }
    int ret_val_2 = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof(opt_val));
    if (ret_val_2 == -1) {
        perror("setsockopt");
        exit(1);
    }

    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }

    if (listen(socket_fd, 100) != 0) {
        perror("listen()");
        exit(1);
    }
    freeaddrinfo(result);
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }
    struct epoll_event ev;
    memset(&ev, '\0', sizeof(struct epoll_event));
    ev.events = EPOLLIN;
    ev.data.fd = socket_fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
        perror("epoll_ctl: socket_fd");
        exit(1);
    }
    
    struct epoll_event events[100];
    while (1) {
       
        int number_events = epoll_wait(epoll_fd, events, 100, -1);
        if (number_events == -1) {
            perror("epoll_wait");
            exit(1);
        }
        int i = 0;
        for (; i < number_events; i++) {
            if (events[i].data.fd == socket_fd) {
                int connection_socket = accept(socket_fd, NULL, NULL);
                if (connection_socket < 0) {
                    perror("accept()");
                    exit(1);
                }
                struct epoll_event ev_conn;
                memset(&ev_conn, '\0', sizeof(struct epoll_event));
                ev_conn.events = EPOLLIN;
                ev_conn.data.fd = connection_socket;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_socket, &ev_conn);
                client_info* info = calloc(1, sizeof(client_info));
                dictionary_set(fd_to_connection_state, &connection_socket, info);
            } else {
                proc_client(events[i].data.fd);
            }
        }
    }
}

int main(int argc, char **argv) {

    if (argc != 2) {
        print_server_usage();
        exit(1);
    }

    signal(SIGPIPE, sp_handle);

    struct sigaction sigint_act;
    memset(&sigint_act, '\0', sizeof(struct sigaction));
    sigint_act.sa_handler = exit_ser;
    int sigaction_result = sigaction(SIGINT, &sigint_act, NULL);
    if (sigaction_result != 0) {
        perror("sigaction");
        exit(1);
    }

    char dirname[] = "XXXXXX";
    dir = mkdtemp(dirname);
    if (dir == NULL)
        exit(1);
    
    print_temp_directory(dir);
    files = string_vector_create();
    fd_to_connection_state = int_to_shallow_dictionary_create();
    server_file_size = string_to_unsigned_long_dictionary_create();
    
    char* port = argv[1];
    run_server(port); 
}
