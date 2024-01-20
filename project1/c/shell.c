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

int handle_command(char** commands, struct Process* bg_tasks);

int shell() {
    char* buffer;
    size_t bufsize = MAX_BUFFER_SIZE; // line of input will contain no more than 128 characters
    size_t characters;
    struct Process* bg_tasks;
    char* prompt;

    // dynamically allocate variables and linked list for background tasks
    bg_tasks = create_process(); 
    buffer = (char *) malloc(bufsize * sizeof(char)); // this allocates 128 bytes of memory. it returns the starting memory address of the string
    prompt = (char *) malloc(MAX_PROMPT_LENGTH * sizeof(char));
   
    set_prompt(prompt, "==>");
    printf("%s", prompt);

    while(getline(&buffer, &bufsize, stdin) > 0) {
        buffer[strcspn(buffer, "\n")] = 0; // Removes newline character
            
        char** commands = split(buffer);

        // handle_command returns -1, we know to terminate the while loop
        int handling_code = handle_command(commands, bg_tasks);
        if (handling_code == -1) {
            break;
        } else if (handling_code == 2) {
            set_prompt(prompt, commands[3]);
        }

        // getting several errors. kind of working. kind of not. 
        
        printf("%s", prompt);
    }

    free(buffer);
    free(prompt);
    return 0;
}

int handle_command(char** commands, struct Process* bg_tasks) {
    int pid;
    struct timeval start, end;
    int background = is_background(commands);
    int returned_process;

    printf("is background: %s\n", background ? "true" : "false");

    // if exit & size() == 0, don't even bother forking
    if(strcmp(commands[0], "exit") == 0) {
        if(size(bg_tasks) == 0) {
            // -1 is the end shell signal
            return -1;
        } else {
            // 0 is the keep going or everything is fine
            printf("background processes still running");
            return 0;
        }
    }

    // tells main shell that this is a default set command, meaning time to update
    if(strcmp(commands[0], "set") == 0) {
        return 2;
    }

    if(background) {
        // remove '&' from command
        removeAmpersand(commands);
    }
 

    // let the child process do its thing
    // make the parent handle the rest

    if((pid = fork()) < 0) {
        fprintf(stderr, "Fork error\n");
        exit(1);
    }
    else if (pid == 0) {
        /* child process */

        // add start time (?)
        // get the current time before the operation
        gettimeofday(&start, NULL);

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

            return 0;
        }
    }
    else {
        // parent

        if(!background) {
            insert(bg_tasks, pid, commands[0]);
            printf("about to visualize\n");
            visualize(bg_tasks);
            print_command(commands);

                returned_process = wait(0);
                printf("got the desired process\n");
                
                // wait til we get the correct pid
                if(returned_process == pid) {
                pop(bg_tasks, returned_process);
                printf("foreground task completed");
                visualize(bg_tasks);
               
            } else if(contains(bg_tasks, returned_process)) {
                pop(bg_tasks, returned_process);
                printf("background task completed");
                visualize(bg_tasks);
            }
            }
        
         else {
        
        // if this is a background task, we need to add this to a list of background tasks
        // this list must be made once and then updated throughout

        /*
            1. make a linked list called background_tasks in shell command
            2. pass the linked list to the handle command
            3. add background tasks to the linked list
            4. repeat

        */

       insert(bg_tasks, pid, commands[0]);;
    }
    
       return 0;
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


int main() {

    shell();

    return 0;
}