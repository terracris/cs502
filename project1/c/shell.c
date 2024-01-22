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

int handle_command(char** commands, struct Process* tasks);
int handle_foreground(char **commands, struct Process *tasks);
int handle_background(char **commands, struct Process *tasks);
void calculate_resources(struct Process* task);
int time_to_millis(long int seconds, long int usec);

int shell() {
    char* buffer;
    size_t bufsize = MAX_BUFFER_SIZE; // line of input will contain no more than 128 characters
    size_t characters;
    struct Process* tasks;
    char* prompt;

    tasks = NULL; //empty linked list

    // dynamically allocated variables
    buffer = (char *)malloc(bufsize * sizeof(char)); // this allocates 128 bytes of memory. it returns the starting memory address of the string
    prompt = (char *) malloc(MAX_PROMPT_LENGTH * sizeof(char));
   
    set_prompt(prompt, "==>");
    printf("%s", prompt);

    while(getline(&buffer, &bufsize, stdin) > 0) {
        buffer[strcspn(buffer, "\n")] = 0; // Removes newline character
            
        char** commands = split(buffer);

        // handle_command returns -1, we know to terminate the while loop
        int handling_code = handle_command(commands, tasks);
        if (handling_code == -1) {
            free(commands);
            break;
        } else if (handling_code == 2) {
            set_prompt(prompt, commands[3]);
        } else if(handling_code == 3) {
            printf("Null pointer exception. Invalid input!");
        }

        // getting several errors. kind of working. kind of not.
        printf("%s", prompt);
    }

    free(buffer);
    free(prompt);
    printf("exited the shell\n");
    return 0;
}

int handle_command(char** commands, struct Process* tasks) {

    int background = is_background(commands);

    printf("is background: %s\n", background ? "true" : "false");

    if(commands[0] == NULL) {
        return 3;
    }

    // if exit & size() == 0, don't even bother forking
    if(strcmp(commands[0], "exit") == 0) {
        if (size(tasks) > 0) {
            printf("Waiting for background processes to finish...\n");

            // Wait for all background processes to finish
            while (size(tasks) > 0) {
                int status;
                pid_t pid = waitpid(-1, &status, 0);

                if (pid > 0) {
                    tasks = delete(tasks, pid);
                    printf("Background process (PID: %d) completed.\n", pid);
                    visualize(tasks);
                }
            }

            printf("All background processes have finished.\n");
        }

        // -1 is the end shell signal
        return -1;
    }

    // tells main shell that this is a default set command, meaning time to update
    if((strcmp(commands[0], "set") == 0) && command_length(commands) == 4) {
        return 2;
    }

    if(strcmp(commands[0], "jobs") == 0) {
        visualize(tasks);
        return 0;
    }

    if(background) {
        // remove '&' from command
        removeAmpersand(commands);
        handle_background(commands, tasks);
    }
    else
    {
        handle_foreground(commands, tasks);
    }

    return 0;
}

int handle_foreground(char** commands, struct Process * tasks) {
    // let the child process do its thing
    // make the parent handle the rest

    // guaranteed to be foreground

    int pid;
    struct timeval start, end;
    int returned_process;

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
                free(commands);
                exit(1);
            }

            return 0;
        }
    }
    else
    {
        // parent
            
            // printf("about to visualize\n");
            // visualize(tasks);
            // print_command(commands);

            printf("Parent process is waiting for child process (PID: %d)\n", pid);

            waitpid(pid, NULL, 0); // waiting for our desired pid
            gettimeofday(&end, NULL);

            // update_end_time(tasks, pid, end);
            visualize(tasks);
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
            // printf("foreground task completed");
            // visualize(tasks);
            
            // after all work with commands is done, only then can we actually free memory
            free(commands);
        }

        return 0;
}

int handle_background(char **commands, struct Process *tasks)
{

    // in this function, we create three processes. everything looks very similar, except we fork again if we are the parent
    // wow, instead of copying and pasting the handle_foreground method, we can essentially just call the handle_foreground

    int pid;

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "Fork error\n");
        exit(1);
    }
    else if (pid == 0)
    {
        handle_foreground(commands, tasks);
        _exit(0);
    } else {
        tasks = insert(tasks, pid, commands[0]);
    }
    
    return 0; // if you are the parent you just return back to the loop

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


void set_prompt(char* prompt, char* phrase) {
    strcpy(prompt, phrase);
}

int is_background(char** commands) {

    int i = 0;
    char* command = commands[i];

    while(command != NULL) {
        if(strcmp(command, "&") == 0) {
            return 1;
        }
        i++;
        command = commands[i];
    }

    // strcmp returns 0 if same. Therefore it is a background task
    return 0;
}

void removeAmpersand(char** commands) {

    int size = command_length(commands);
    commands[size - 1] = NULL;
}

int command_length(char** commands) {

    int i = 0;
    while(commands[i] != NULL) {
        i++;
    }

    return i;

}

void print_command(char** commands) {
    int i = 0;

    while(commands[i] != NULL) {
        printf("%s ", commands[i]);
        i++;
    }
    printf("\n");
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

void calculate_resources(struct Process* task) {

    
}


int main() {

    shell();

    return 0;
}