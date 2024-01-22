#ifndef PROCESS
#define PROCESS

#define UNDEFINED_PROCESS_IDENTIFIER -1

#include <sys/time.h>

// Linked List declaration for background tasks
struct Process {
    int pid;
    char* taskName;
    struct Process * next;  
};

/* prototypes */
void init(struct Process * tasks);
struct Process* create_process();
struct Process* insert(struct Process* tasks, int pid, char* command);
struct Process* get(struct Process* tasks, int pid);
struct Process* pop(struct Process* tasks, int pid);
int contains(struct Process* tasks, int pid);
struct Process *delete(struct Process *tasks, int pid);
// struct Process* update_end_time(struct Process* tasks, int pid, struct timeval end);
int size(struct Process* bg_tasks);
void visualize(struct Process* tasks);

#endif