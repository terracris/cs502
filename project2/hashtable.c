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

/* From now on, insert handles checking if it contains.
   We the user do not care about how values are incremented in this
   Hashtable. Its almost like a histogram.*/


   /* Man, I think the previous assignment confused me a little on my view on memory
      If memory already is allocated on the heap, I don't need to keep assigning a variable to it.
      
      Let me explain why my confusion arose. I had a variable in the stack that was not updating
      the value of the Linked List. I believe if I passed the memory address of that linked list
      variable, I would not have had this problem.  */
void insert(HashTable* ht, const char* key, const char* subsequent) {
    unsigned int index = hash(key);
    KeyValuePair* current = ht->table[index];

    // if the value already exists in the hashtable just increment the value
    if(current != NULL) {
        current->value++;
        add_sequential(current, subsequent);
        return;
    }

    // Create a new key-value pair --> calling malloc bc we want the
    // value to exist outside of the function call
    
    // if the value is not already in the hashtable we add it
    KeyValuePair* new_pair = malloc(sizeof(KeyValuePair));
    new_pair->key = strdup(key); // this allocates space for this string so need to free
    new_pair->value = 1;
    add_sequential(new_pair, subsequent);

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

Sequential * create_sequential(const char* call) {
        Sequential* new_sequential = (Sequential *) malloc(sizeof(Sequential)); // allowcates a sequential call node
        new_sequential->call = strdup(call);
        new_sequential->count = 1;
        new_sequential->next = NULL;

        return new_sequential;
}

// unlikely this function will ever be called with a null KeyValuePair *
void add_sequential(KeyValuePair* current, const char* call) {
    
    // if the call is NULL, there is nothing to add, so we can just return
    if(call == NULL) {
        return;
    }

    Sequential* head = current->sequential;

    if(head == NULL) {
        // this is the first sequential call
        Sequential* new_sequential = create_sequential(call);
        current->sequential = new_sequential; // adds sequential to KeyValuePair
        return;
    }
    
    while(head->next != NULL) {
        if(strcmp(head->call, call) == 0) {
            // we have found the call
            head->count++; // therefore we increase the count and return
            return;
        }

        head = head->next; // keep looking until we find it
    }

    // if we could not find it, we append it to the list
    Sequential* new_sequential = create_sequential(call);
    head->next = new_sequential;

}

// this could be the visualization for the sequential
void visualize_sequential(KeyValuePair* current) {
    
    Sequential* head = current->sequential;

    while(head != NULL) {
        printf("%s:%s %d\n", current->key, head->call, head->count);
        head = head->next;
    }

}

// checks to see if a key already exists in the Hashtable
// pass by reference is best for updatings things in place
// pass by value creates a copy of that data and does not
// actually update the final table
// returns true if get does not return -1
int contains(HashTable* ht, const char* key) {

    int index = hash(key);

    return ht->table[index] != NULL;

}

void freeHashTable(HashTable* ht) {
    // first we remove all subsequent calls
    for(int i = 0; i < TABLE_SIZE; i++) {
        if(ht->table[i] != NULL) {
            free_subsequent_calls(ht->table[i]);
            free(ht->table[i]->key);
            free(ht->table[i]);
        }
    }
}

// LOL I find it kind of funny that you have to rememeber the memory you allocated, 
// but at the same time it is pretty cool
void free_subsequent_calls(KeyValuePair* pair) {
    
        Sequential* current = pair->sequential;

        while (current != NULL) {
            Sequential* next = current->next; // stores the next item before the current is freed.
            free(current->call); // gotta free the allocated strings
            free(current);
            current = next;
        }

}

void visualize(HashTable* ht) {
    for(int i = 0; i < TABLE_SIZE; i++) {
        if(ht->table[i] != NULL) {
            printf("%s ", ht->table[i]->key);
            printf("%d\n", ht->table[i]->value);
            visualize_sequential(ht->table[i]);
        }
    }
}

KeyValuePair * getPair(HashTable* ht, char *key) {
    int index = hash(key);

    return ht->table[index];
}