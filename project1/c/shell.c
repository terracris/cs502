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
void calculate_resources(struct Process* task);
int time_to_millis(long int seconds, long int usec);
struct Process* handle_task(char** commands, struct Process * tasks);

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
    int exit = 0;  // prompts exit flag


    while((getline(&buffer, &bufsize, stdin) > 0) && !exit) {
        buffer[strcspn(buffer, "\n")] = 0; // Removes newline character
            
        char** commands = split(buffer);

        // handle_command returns 2, we know to terminate the while loop
        int handling_code = handle_command(commands, tasks);

        // I will make two methods for handling input. one for updating task list, and one for updating simple commands
        switch(handling_code) {
            case 0:
            printf("jobs listed");
            break;

            case 1:
            // set new prompt
            set_prompt(prompt, commands[3]);
            break;

            case 2:
            exit = 1; // creates exit flag
            // waits for all tasks to complete
            tasks = handle_task(commands, tasks);
            break;

            case -1:
            tasks = handle_task(commands, tasks);
            break;
        }

        free(commands);
        // getting several errors. kind of working. kind of not.
        printf("%s", prompt);
    }

    free(buffer);
    free(prompt);
    printf("exited the shell\n");
    return 0;
}

int handle_command(char** commands, struct Process* tasks) {

    // tells main shell that this is a default set command, meaning time to update
    if((strcmp(commands[0], "set") == 0) && command_length(commands) == 4) {
        return 0;
    }

    if(strcmp(commands[0], "jobs") == 0) {
        visualize(tasks);
        return 1;
    }

    if(strcmp(commands[0], "exit") == 0) {
        return 2;
    }

    return -1;
}

struct Process* handle_task(char** commands, struct Process * tasks) {
    // let the child process do its thing
    // make the parent handle the rest

    int background = is_background(commands);

    printf("is background: %s\n", background ? "true" : "false");

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
        return tasks;
    }

    if(background)
        removeAmpersand(commands);

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
        // parent
            
            // printf("about to visualize\n");
            // visualize(tasks);
            // print_command(commands);
            int returned_process;

            if (!background) {
                // If not a background, wait for the current pid,
                // if a background task completes, we should remove it from the linked list
                while(returned_process = wait(0)) {

                    if(returned_process == pid) {
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
                        
                    } else if(contains(tasks, returned_process)) {
                        tasks = delete(tasks, pid); // only time to delete from the linked list
                    }

                }
            } else {
                // if it is a background task, just insert it and return
                tasks = insert(tasks, pid, commands[0]);
                visualize(tasks);
                return tasks;
            }
            
}
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