#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

#define MAX_COMMANDS 32
#define MAX_PROMPT_LENGTH 16

/* prototypes */
int time_to_millis(long int seconds, long int usec);
int handle_command(char** commands);
int shell();
char** split(char* command);

int main(int argc, char **argv) {
    if (argc > 1) {
        printf("whoops");
    }
    else {
        shell();
    }
}

int shell() {
    char* buffer;
    size_t bufsize = 128; // line of input will contain no more than 128 characters
    size_t characters;

    // malloc returns a pointer! (memory address --> you must cast it to the correct type)
    buffer = (char *) malloc(bufsize * sizeof(char)); // this allocates 128 bytes of memory. it returns the starting memory address of the string
    if (buffer == NULL) {
        perror("Unable to allocate buffer");
        exit(1);
    }

    char * prompt = (char *) malloc(MAX_PROMPT_LENGTH * sizeof(char));
    strcpy(prompt, "==>");

    printf("%s", prompt);
    while(getline(&buffer, &bufsize, stdin) > 0) {
        buffer[strcspn(buffer, "\n")] = 0; // Removes newline character

        // TODO: refactor to switch case.
        
        if(strcmp(buffer, "exit") == 0)
            break;
        
        char** commands = split(buffer);
        
        if(strcmp(commands[0], "set") == 0) {
            strcpy(prompt, commands[3]);
        }

        if(strcmp(commands[0], "cd") == 0) {
            if(chdir(commands[1]) != 0) {
                perror("Could not change directory");
            } else {
                // this is fine for now. An improvement would be using pwd command
                printf("Current directory is changed to %s\n", commands[1]);
            }
        } else {
            handle_command(commands);
        }

        
        printf("%s", prompt);
    }

    free(buffer);
    free(prompt);
    return 0;
}

char** split(char* command) {
    
    // creates an array of strings (an array of array of characters)
    char** commands = (char**) malloc((MAX_COMMANDS + 1)* sizeof(char*)); // + 1 for NULL terminator
    char* token = strtok(command, " ");

    // loop through the string to extract all other tokens
    int i = 0;
    while(token != NULL) {
        commands[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    
    commands[i] = NULL; // terminates after the last token

    for (int j = 0; j < i; j++)
    {
        printf("Current command: '%s'\n", commands[j]);
    }
    
    return commands;
}

int handle_command(char** commands) {
    int pid;
    struct timeval start, end;

    // get the current time before the operation

    gettimeofday(&start, NULL);

    if((pid = fork()) < 0) {
        fprintf(stderr, "Fork error\n");
        exit(1);
    }
    else if (pid == 0) {
        // child process
        int i = 0;
        
        if(execvp(commands[0], commands) < 0 ) {
            fprintf(stderr, "Execute error\n");
            free(commands);
            exit(1);
        }

        free(commands);
        exit(0);
    }
    else {
        // parent
        wait(0);
        gettimeofday(&end, NULL);

        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        int elapsed = time_to_millis(seconds, microseconds);

        struct rusage usage;
        if (getrusage(RUSAGE_CHILDREN, &usage) == 0) {
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

        return 0;

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