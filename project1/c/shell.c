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


int shell() {
    char* buffer;
    size_t bufsize = MAX_BUFFER_SIZE; // line of input will contain no more than 128 characters
    size_t characters;
    struct Process* bg_tasks;
    char* prompt;

    // dynamically allocate variables and linked list for background tasks
    bg_tasks = (struct Process*) malloc(sizeof(struct Process));
    buffer = (char *) malloc(bufsize * sizeof(char)); // this allocates 128 bytes of memory. it returns the starting memory address of the string
    prompt = (char *) malloc(MAX_PROMPT_LENGTH * sizeof(char));
   
    set_prompt(prompt, "==>");
    initialize_tasks(bg_tasks); // sets certain values to NULL

    printf("%s", prompt);

    while(getline(&buffer, &bufsize, stdin) > 0) {
        buffer[strcspn(buffer, "\n")] = 0; // Removes newline character
            
        char** commands = split(buffer);
        handle_command(commands, bg_tasks);
        
        printf("%s", prompt);
    }

    free(buffer);
    free(prompt);
    return 0;
}

int can_exit(struct Process* tasks) {

    return calculate_num_tasks(tasks) == 0;
}

int is_background(char** commands) {
    int length = strlen(commands); // the array is null terminated so I am hoping this works

    // strcmp returns 0 if same. Therefore it is a background task
    return !strcmp(commands[length - 1], "&");
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

void temp() {
    if(strcmp(buffer, "exit") == 0) {
            if (can_exit(bg_tasks))
                break;
            else
                printf("waiting");

        }
}

int handle_command(char** commands, struct Process* bg_tasks) {
    int pid;
    struct timeval start, end;
    int background = is_background(commands);


    if((pid = fork()) < 0) {
        fprintf(stderr, "Fork error\n");
        exit(1);
    }
    else if (pid == 0) {
        /* child process */
    
        // get the current time before the operation
        gettimeofday(&start, NULL);
        int i = 0;

        if(strcmp(commands[0], "cd") == 0) {
            if(chdir(commands[1]) != 0) {
                perror("Could not change directory");
            } else {
                // this is fine for now. An improvement would be using pwd command
                printf("Current directory is changed to %s\n", commands[1]);
            }
        } else {
            if (execvp(commands[0], commands) < 0 ) {
            fprintf(stderr, "Execute error\n");
            free(commands);
            exit(1);
            }
        }
    }
    else {
        // parent

        if(!background) {
            int returned_process = wait(0); // wait(0) will give us what we want
            while(1) {
                if(returned_process == pid) {
                // print the stats for the pid and then we return
                break;
            } else if(contains(bg_tasks, returned_process)) {
                pop(bg_tasks, returned_process);
            }
            }
            
            
    } else {
        
        // if this is a background task, we need to add this to a list of background tasks
        // this list must be made once and then updated throughout

        /*
            1. make a linked list called background_tasks in shell command
            2. pass the linked list to the handle command
            3. add background tasks to the linked list
            4. repeat

        */

       add_bg_task(bg_tasks, pid, commands[0], start);
    }
    
       return 0;
    }
}

void update_on_set(char** commands, char* prompt) {

 if(strcmp(commands[0], "set") == 0) {
            strcpy(prompt, commands[3]);
        }
}