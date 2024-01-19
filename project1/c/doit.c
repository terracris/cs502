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



/* prototypes */
int time_to_millis(long int seconds, long int usec);
int handle_command(char** commands, struct Process * bg_tasks);
int shell();
void update_on_set(char** commands, char* prompt);
char** split(char* command);
void initialize_tasks(struct Process * tasks);
int can_exit(struct Process* tasks);
void add_bg_tasks(struct Process* tasks,int pid, char* command, struct timeval start);



int main(int argc, char **argv) {
    if (argc > 1) {
        printf("whoops");
    }
    else {
        shell();
    }
}


void print_stats(struct timeval end) {
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




/**
 * @brief calculates the number of milliseconds elapsed based ont he timeval
 * 
 * @param tv_sec number of whole seconds of elapsed time
 * @param tv_usec number of microseconds elapsed
 * @return int number of milliseconds elapsed
 */

int time_to_millis(long int seconds, long int usec) {
    
    /* TODO link math library to use ceil for time calculations */
    int result = (seconds * 1000) + (int)((float)usec / 1000.0); // implicit conversion to milliseconds

    return result;
}
