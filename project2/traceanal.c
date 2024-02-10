#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashtable.h"

#define MAX_SYSTEM_CALL_NAME_LENGTH 100

int main(int argc, char** argv) {
    char buffer[1024]; // Adjust the buffer size as needed
    HashTable myTable;
    initHashTable(&myTable);
    int total_calls = 0;
    int unique_calls = 0;
    char *previousKey = NULL;
    int seq = 0;

    if (argc > 1 && (strcmp(argv[1], "seq") == 0)) {
        seq = 1; // if we have more than the program being called, then we print the rest
    }


    // Read input from stdin
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Process each line of input here
        // For example, you can print the lines for demonstration

        // if the character exists in a given line
        if(strchr(buffer, '(') != NULL) {
            
            // only then can we process it
            char* currentKey = strtok(buffer, "(");
            
            if (previousKey != NULL) {
                insert(&myTable, previousKey, currentKey);
                previousKey = strcpy(previousKey, currentKey);
                
            } else {
                previousKey = (char *) malloc(sizeof(char) * MAX_SYSTEM_CALL_NAME_LENGTH);
                previousKey = strcpy(previousKey, currentKey);
            }

            if(!contains(&myTable, currentKey)) {
                unique_calls++;
            }
            

         total_calls++;

        } 
    }

    // need to insert the last item
    insert(&myTable, previousKey, NULL);
    printf("AAA: %d invoked system call instances from %d unique system calls.\n", total_calls, unique_calls);
    visualize(&myTable, seq);
    freeHashTable(&myTable);
    free(previousKey);

    return 0;
}
