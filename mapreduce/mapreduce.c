/**
 * mapreduce
 * CS 341 - Fall 2023
 */
#include "utils.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>



int main(int argc, char **argv) {
    // Create an input pipe for each mapper.
    char* inpfile = argv[1];
    char* outfile = argv[2];
    char* mapper = argv[3];
    char* reducer = argv[4];
    int mapcount;
    sscanf(argv[5], "%d", &mapcount);
    int* fd[mapcount];
    int i = 0;
    for (;i< mapcount; i++) {
        fd[i] = calloc(2, sizeof(int));
        pipe(fd[i]);
    }
    // Create one input pipe for the reducer.
    int fdreduce[2];
    pipe(fdreduce);
    // Open the output file.
    int openfile = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR);

    // Start a splitter process for each mapper.
    pid_t childsplit[mapcount];
    i = 0;
    for (; i< mapcount; i++) {
        childsplit[i] = fork();
        if (childsplit[i] == 0) {
            close(fd[i][0]);
            char temp[16];
            sprintf(temp, "%d", i);
            dup2(fd[i][1], 1);
            execl("./splitter", "./splitter", inpfile, argv[5], temp, NULL);
            exit(1);
        }
    }
    // Start all the mapper processes.
    pid_t childmap[mapcount];
    i = 0;
    for (; i < mapcount; i++) {
        close(fd[i][1]);
        childmap[i] = fork();
        if (childmap[i] == 0) {
            close(fdreduce[0]);
            dup2(fd[i][0], 0);
            dup2(fdreduce[1], 1);
            execl(mapper, mapper, NULL);
            exit(1);
        }
    }
    // Start the reducer process.
    close(fdreduce[1]);
    pid_t child = fork();
    if (child == 0) {
        dup2(fdreduce[0], 0);
        dup2(openfile, 1);
        execl(reducer, reducer, NULL);
        exit(1);
    }
    close(openfile);
    close(fdreduce[0]);

    // Wait for the reducer to finish.
    i = 0;
    for (; i < mapcount; i++) {
        int s;
        waitpid(childsplit[i], &s, 0);
    } 
    i = 0;
    for (; i < mapcount; i++) {
        close(fd[i][0]);
        int s;
        waitpid(childmap[i], &s, 0);
    }
    int s;
    waitpid(child, &s, 0);
    // Print nonzero subprocess exit codes.
    if (s)
        print_nonzero_exit_status(reducer, s);
    

    // Count the number of lines in the output file.
    print_num_lines(outfile);
    i = 0;
    for (; i< mapcount; i++)
        free(fd[i]);
    
    return 0;
}
