#ifndef PROCESS
#define PROCESS

#include <sys/time.h>

// Linked List declaration for background tasks
struct Process {
    int pid;
    char* taskName;
    struct timeval start;
    struct timeval end;
    struct Process * next;  
};

#define UNDEFINED_PROCESS_IDENTIFIER -1


#endif