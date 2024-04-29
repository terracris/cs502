#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "shell.h"
#include "process.h"

char** convert_input(int argc, char** argv);
int handle_input(char** commands);

int main(int argc, char **argv) {
    if (argc > 1) {
        char** commands = convert_input(argc, argv);
        handle_input(commands);
        free(commands);
    }
    else {
        shell();
    }
    return 0;
}

char** convert_input(int argc, char** argv) {

    char** commands = (char**) malloc((MAX_COMMANDS + 1)* sizeof(char*)); // + 1 for NULL terminator
    
    int i;

    for(i = 0; i < argc - 1; i++) {
        commands[i] = argv[i + 1]; //copying elements of argv
    }

    commands[i] = NULL;

    return commands;
}

int handle_input(char** commands) {
    
    int pid;
    struct timeval start, end;

     // get the current time before the operation
    gettimeofday(&start, NULL);

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "Fork error\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* child process */

        if (strcmp(commands[0], "cd") == 0)
        {
            if (chdir(commands[1]) != 0)
            {
                perror("Could not change directory");
            }
            else
            {
                // this is fine for now. An improvement would be using pwd command
                printf("Current directory is changed to %s\n", commands[1]);
            }
        }
        else
        {
            if (execvp(commands[0], commands) < 0)
            {
                fprintf(stderr, "Execute error\n"); 
                exit(1);
            }

            return 0;
        }
    }
    else
    {
        wait(0);
        // print stats if foreground task completed
        gettimeofday(&end, NULL);
        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        int elapsed = time_to_millis(seconds, microseconds);

        struct rusage usage;
        if (getrusage(0, &usage) == 0) {
        printf("User CPU time (milliseconds): %d\n", time_to_millis(usage.ru_utime.tv_sec, usage.ru_utime.tv_usec));
        printf("System CPU time (milliseconds): %d\n", time_to_millis(usage.ru_stime.tv_sec, usage.ru_stime.tv_usec));
        printf("elapsed time (milliseconds): %d\n", elapsed);
        printf("# times process was preempted involuntarily: %ld\n", usage.ru_nivcsw);
        printf("# times process was preempted voluntarily: %ld\n", usage.ru_nvcsw);
        printf("# of major page faults: %ld\n", usage.ru_majflt);
        printf("# of minor page faults: %ld\n", usage.ru_minflt);
        printf("max resident set size used: %ld\n", usage.ru_maxrss);
        } else {
            fprintf(stderr, "Could not get usage statistics\n");
            exit(1);
        }
    }
}
