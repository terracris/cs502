#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

// int main() {
//     // so the variable myTable is a Hashtable stored on the stack,
//     // the entries of the table (key-value pairs) are stored on the
//     // heap. so they manually need to be destroyed
//     HashTable myTable;


//     // calling &myTable gives a pointer to a hash table on the stack
//     // &mytable is still just like any other pointer
//     // therefore we are able to change the values in place
//     // i want to try this with my linkedlist implementation
//     // and see if I could improve it. 
//     initHashTable(&myTable);

//     const char* key1 = "apple";
//     const char* key2 = "banana";
//     const char* key3 = "orange";

//     // typically you should always pass complex values
//     // by reference rather than value

//     for(int i = 0; i < 5; i++) {
//         if(contains(&myTable, key1)) {
//             unsigned int updated_count = get(&myTable, key1) + 1;
//             insert(&myTable, key1, updated_count);
//         }  else {
//             insert(&myTable, key1, 1);
//         }
//     }

//     printf("Value for 'apple': %d\n", get(&myTable, key1));
//     freeHashTable(&myTable);

//     return 0;
// }

// Hash function to generate an index from the key
unsigned int hash(const char* key) {
    unsigned int hash_value = 0;

    /* this fun syntax is saying "while key is not null" 
       this while loop is checking that the character pointed to by key
       is not null. then it dereferences it and increments the value key
       points to. It kinda reads the way you see it from left to right.     
    */

    while(*key) {
        hash_value = (hash_value << 5) + *key++;
    }

    return hash_value % TABLE_SIZE;
}

void initHashTable(HashTable* ht) {
    for(int i = 0; i < TABLE_SIZE; i++) {
        ht->table[i] = NULL;
    }
}

void insert(HashTable* ht, const char* key, int value) {
    unsigned int index = hash(key);

    // Create a new key-value pair --> calling malloc bc we want the
    // value to exist outside of the function call

    KeyValuePair* new_pair = malloc(sizeof(KeyValuePair));
    new_pair->key = strdup(key); // this allocates space for this string so need to free
    new_pair->value = value;

    // insert the pair into the hash table
    ht->table[index] = new_pair;
}

// Function to retrieve the value associated with a key
int get(HashTable* ht, const char* key) {
    unsigned int index = hash(key);

    // Retrieve the pair from the hash table
    KeyValuePair* pair = ht->table[index];

    // Check if the key exists in the hash table
    if(pair != NULL && strcmp(pair->key, key) == 0) {
        return pair->value;
    }

    // Key not found
    return -1;
}

// checks to see if a key already exists in the Hashtable
// pass by reference is best for updatings things in place
// pass by value creates a copy of that data and does not
// actually update the final table
// returns true if get does not return -1
int contains(HashTable* ht, const char* key) {

    return get(ht, key) != -1;

}

void freeHashTable(HashTable* ht) {
    for(int i = 0; i < TABLE_SIZE; i++) {
        if(ht->table[i] != NULL) {
            free(ht->table[i]->key);
            free(ht->table[i]);
        }
    }
}

void visualize(HashTable* ht) {
    for(int i = 0; i < TABLE_SIZE; i++) {
        if(ht->table[i] != NULL) {
            printf("%s ", ht->table[i]->key);
            printf("%d\n", ht->table[i]->value);
        }
    }
}