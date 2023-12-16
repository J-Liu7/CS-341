/**
 * deepfried_dd
 * CS 341 - Fall 2023
 */
#include "format.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

static int pr_statistic = 0;
void sig_handle(int sig) {
    if (sig == SIGUSR1)
        pr_statistic = 1;
    
}


int main(int argc, char **argv) {
    signal(SIGUSR1, sig_handle);
    int opt = 0;
    FILE* fpin = stdin;
    FILE* fpout = stdout;
    long skip_ins = 0;
    long skip_outs = 0;
    long block_copies = 0;
    long block_size = 512;
    while((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch(opt) {
            case 'i':
                fpin = fopen(optarg, "r");
                if (fpin == NULL) {
                    print_invalid_input(optarg);
                    exit(1);
                }
                continue;
            case 'o':
                fpout = fopen(optarg, "w+");
                if (fpout == NULL) {
                    print_invalid_output(optarg);
                    exit(1);
                }
                continue;
            case 'b':
                block_size = atol(optarg);
                continue;
            case 'c':
                block_copies = atol(optarg);
                continue;
            case 'p':
                skip_ins = atol(optarg);
                continue;
            case 'k':
                skip_outs = atol(optarg);
                continue;
            case '?':
                exit(1);
        }
    }
    fseek(fpin, skip_ins, SEEK_SET);
    fseek(fpout, skip_outs, SEEK_SET);
    clock_t before = clock();
    size_t fb_in = 0;
    size_t pb_in = 0;
    size_t c_size = 0;
    while (1) {
        if (feof(fpin))
            break;
        
        if (block_copies != 0 && pb_in + fb_in == (unsigned long) block_copies)
            break;
        
        if (pr_statistic) {
            clock_t difference = clock() - before;
            double time_spent_ = 1000* difference / CLOCKS_PER_SEC;
            time_spent_ /= 1000;

            print_status_report(fb_in, pb_in,
                        fb_in, pb_in,
                        c_size, time_spent_);
            pr_statistic = 0;
        }
        char buffer[block_size];
        size_t reads = fread((void*) buffer, 1, block_size, fpin);
        if (reads == 0) {
            break;
        }
        if (reads >= (unsigned long) block_size) {
            fflush(stdin);
            fwrite((void*) buffer, block_size, 1, fpout);
            fb_in++;
            c_size += block_size;
        } else {
            pb_in++;
            c_size += reads;
            fwrite((void*) buffer, reads, 1, fpout);
        }
        
    }
    clock_t diff = clock() - before;
    long double time_spent = 1000* diff / CLOCKS_PER_SEC;
    time_spent /= 1000;
    print_status_report(fb_in, pb_in,
                        fb_in, pb_in,
                        c_size, time_spent);
    return 0;
}