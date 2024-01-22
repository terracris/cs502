#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

// implementation of this linked list is ASS
// you need to first declare the linked list first as just null
// and then once you want to insert you check if the task list is null

/*

Currently the IDE is warning me of memory leaks and incorrect assumptions I am making in my code that is not guaranteed.

*/

struct Process *create_process()
{
    struct Process *new = (struct Process *)malloc(sizeof(struct Process));

    if (new == NULL)
    {
        printf("Could not allocate memory\n");
        exit(1);
    }

    return new;
}

struct Process *insert(struct Process *tasks, int pid, char *command)
{
    // check if the first item has been assigned.
    // if no first process, then we assign it.

    if (tasks == NULL)
    {
        struct Process *new = create_process(); // dynamically allocates struct process
        new->pid = pid;
        new->taskName = strdup(command); // copy the name of the command
        return new;
    }

    // add element to linked list
    struct Process *curr = tasks;

    while (curr->next != NULL)
    {
        curr = curr->next;
    }

    struct Process *new = create_process();
    new->pid = pid;
    new->taskName = strdup(command);
    curr->next = new;

    return tasks;
}

// gets the process with a specific pid
struct Process *get(struct Process *tasks, int pid)
{
    // if you try to remove a pid from the linked list when there are 0 items, return null
    if (tasks == NULL)
    {
        return NULL;
    }

    struct Process *curr = tasks;

    while (curr != NULL)
    {

        if (curr->pid == pid)
        {
            struct Process *temp = curr;
            return temp;
        }

        curr = curr->next;
    }

    // return NULL if we could not find PID
    return NULL;
}

// removes a memory address

struct Process *delete(struct Process *tasks, int pid)
{
    // if you try to remove a pid from the linked list when there are 0 items, return null
    if (tasks == NULL)
    {
        return NULL;
    }

    // if it is the first item, make
    if (tasks->pid == pid)
    {
        struct Process *temp = tasks;
        tasks = tasks->next;
        free(temp); // free the removed memory address.

        return tasks;
    }

    struct Process *prev = tasks;
    struct Process *curr = tasks;

    while (curr != NULL)
    {
        if (curr->pid == pid)
        {
            struct Process *temp = curr;
            prev->next = curr->next;
            free(temp);
            return tasks;
        }

        prev = curr;
        curr = curr->next;
    }

    // return NULL if we could not find PID
    return NULL;
}

int contains(struct Process *tasks, int pid)
{

    // empty linked list
    if (tasks == NULL)
    {
        return 0;
    }

    struct Process *curr = tasks;

    while (curr != NULL)
    {

        if (curr->pid == pid)
        {
            // if pid found, yes it contains
            return 1;
        }

        curr = curr->next;
    }

    // return 0 if we could not find PID
    return 0;
}

struct Process *update_end_time(struct Process *tasks, int pid, struct timeval end)
{

    struct Process *tmp = get(tasks, pid);

    // process did not exist
    if(tmp == NULL) {
        return NULL;
    }
    
    tmp->end = end;

    // returning in case we want to calculate the data on that one item
    // not sure if timeval should be a memory address or not
    // I will have to ask chatgpt or professor during office hours
    return tmp;
}

int size(struct Process *bg_tasks)
{
    int num_tasks = 0;
    struct Process *curr = bg_tasks;

    while (curr != NULL)
    {
        num_tasks++;
        curr = curr->next;
    }

    return num_tasks;
}

void visualize(struct Process *tasks)
{

    struct Process *curr = tasks;

    while (curr != NULL)
    {
        printf("pid: %d (%s) --> ", curr->pid, curr->taskName);
        curr = curr->next;
    }

    printf("\n");
}
