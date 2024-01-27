#include <stdio.h>
#include <string.h>
#include "hashtable.h"

int main() {
    char buffer[1024]; // Adjust the buffer size as needed
    HashTable myTable;
    initHashTable(&myTable);
    int total_calls = 0;
    int unique_calls = 0;
    char *previousKey = NULL;
    KeyValuePair* prev = NULL;


    // Read input from stdin
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Process each line of input here
        // For example, you can print the lines for demonstration

        // if the character exists in a given line
        if(strchr(buffer, '(') != NULL) {
            
            // only then can we process it
            char* key = strtok(buffer, "(");
            if(contains(&myTable, key)) {
                unsigned int updated_count = get(&myTable, key) + 1;
                insert(&myTable, key, updated_count);

                if(previousKey != NULL) {
                    insert(prev->sequential, key, get(prev->sequential, key) + 1) 
                }
         }  else {
                insert(&myTable, key, 1);
                unique_calls++;
         }

         total_calls++;
         prev = getPair(&myTable, key);

        } 
        
    }
    printf("AAA: %d invoked system call instances from %d unique system calls.\n", total_calls, unique_calls);
    visualize(&myTable);
    freeHashTable(&myTable);

    return 0;
}
