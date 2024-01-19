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
int handle_command(char** commands, struct Process * bg_tasks);
int shell();

// Linked List declaration for background tasks

struct Process {
    int pid;
    char* taskName;
    struct timeval start;
    struct timeval end;
    struct Process * next;  
};

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
    struct Process* bg_tasks;

    // dynamically allocated linked list for background tasks
    bg_tasks = (struct Process*) malloc(sizeof(struct Process));
    initialize_tasks(bg_tasks); // sets certain values to NULL

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

        if(strcmp(buffer, "exit") == 0) {
            if (can_exit(bg_tasks))
                break;
            else
                printf("waiting");

        }
            
        char** commands = split(buffer);
        update_on_set(commands, prompt);
        handle_command(commands, bg_tasks);
        
        printf("%s", prompt);
    }

    free(buffer);
    free(prompt);
    return 0;
}