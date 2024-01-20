#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

void init(struct Process * tasks) {
    tasks->pid = UNDEFINED_PROCESS_IDENTIFIER;
    tasks->taskName = NULL;
    tasks->next = NULL;
}

struct Process* create_process() {
    struct Process* new = (struct Process *) malloc(sizeof(struct Process));
    
    if(new == NULL) {
        printf("Could not allocate memory\n");
        exit(1);
    }

    init(new);

    return new;
}

struct Process* insert(struct Process* tasks, int pid, char* command) {
    // check if the first item has been assigned. 
    // if no first process, then we assign it. 

    if (tasks->pid == UNDEFINED_PROCESS_IDENTIFIER)
    {
       tasks->pid = pid;
       tasks->taskName = strdup(command); // copy the name of the command
       return tasks;
    }

    // add element to linked list
    struct Process* curr = tasks;

    while(curr->next != NULL) {
        curr = curr->next;
    }

    struct Process* new = create_process();
    new->pid = pid;
    new->taskName = strdup(command);
    curr->next = new;

    return tasks;
    
}

// gets the process with a specific pid
struct Process* get(struct Process* tasks, int pid) {
    // if you try to remove a pid from the linked list when there are 0 items, return null
    if(tasks->pid == UNDEFINED_PROCESS_IDENTIFIER) {
        return NULL;
    }

    struct Process* curr = tasks;
    
    while(curr!= NULL) {
        
        if(curr->pid == pid) {
            struct Process* temp = curr;
            return temp;
        }

        curr = curr->next;
    }

    // return NULL if we could not find PID
    return NULL;
}

// removes and returns the memory address of the removed item
// technically, this works as a get and remove method. 
// TODO: Make a get() and remove() function and test it out.

struct Process* pop(struct Process* tasks, int pid) {
    // if you try to remove a pid from the linked list when there are 0 items, return null
    if(tasks->pid == UNDEFINED_PROCESS_IDENTIFIER) {
        return NULL;
    }

    struct Process* prev = tasks;
    struct Process* curr = tasks;
    
    while(curr!= NULL) {
        if(curr->pid == pid) {
            struct Process* temp = curr;
            prev->next = curr->next;
            temp->next = NULL;
            return temp;
        }

        prev = curr;
        curr = curr->next;
    }

    // return NULL if we could not find PID
    return NULL;
}

int contains(struct Process* tasks, int pid) {
    
    // empty linked list
    if(tasks->pid == UNDEFINED_PROCESS_IDENTIFIER) {
        return 0;
    }

    struct Process* curr = tasks;
    
    while(curr!= NULL) {
        
        if(curr->pid == pid) {
            // if pid found, yes it contains
            return 1;
        }

        curr = curr->next;
    }

    // return 0 if we could not find PID
    return 0;
}

void delete(struct Process* tasks) {

    struct Process* curr = tasks;

    for(int i = 0; i < size(tasks); i++) {
        struct Process* tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    printf("succesfully freed the item\n");
}

struct Process* update_end_time(struct Process* tasks, int pid, struct timeval end) {

    struct Process* tmp = get(tasks, pid);
    tmp->end = end;

    // returning in case we want to calculate the data on that one item
    // not sure if timeval should be a memory address or not
    // I will have to ask chatgpt or professor during office hours
    return tmp;
}


int size(struct Process* bg_tasks) {
    int num_tasks = 0;
    struct Process* curr = bg_tasks;

    while(curr->pid != UNDEFINED_PROCESS_IDENTIFIER) {
        num_tasks++;
        curr = curr->next;
    }

    return num_tasks;
}

void visualize(struct Process* tasks) {

    struct Process* curr = tasks;

    while(curr != NULL) {
        printf("pid: %d (%s) --> ", curr->pid, curr->taskName);
        curr = curr->next;
    }

    printf("\n");
}




