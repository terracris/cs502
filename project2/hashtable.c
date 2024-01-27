#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

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

KeyValuePair * getPair(HashTable* ht, char *key) {
    int index = hash(key);

    return ht->table[index];
}