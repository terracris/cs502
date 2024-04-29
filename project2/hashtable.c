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
    unsigned int starting_index = index;
    KeyValuePair* current = ht->table[index];

    // if the value already exists in the hashtable just increment the value
    // I am very proud to say that I have figured out my bug with my program.
    // hashing is not perfect. Sometimes keys hash to the same hash as another key already exisiting 
    // in the hash table. therefore, you must check for string equality. if not, we should looking
    // for the next existing location. 
    
    // linear probing we learned in algorithms. look for the next available spot 
    // --> what is the end condition for probing symbol 
    
    // we keep track of the starting index, increment and modulo until
    // we get back to the starting index. if that is the case, we know our value does not exist
    while(current != NULL)  {
        // if current becomes NULL, we do have space for it given at index
        if(strcmp(current->key, key) == 0) {
            current->value++;
            add_sequential(current, subsequent);
            return;
        }

        index++;
        index = index % TABLE_SIZE;
        current = ht->table[index];

        if(index == starting_index) {
            printf("No more room in table");
            return;
        }
        
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
    int starting_index = index;
    KeyValuePair* pair = ht->table[index];

    // looking for the value
    while(pair != NULL) {
        if(strcmp(pair->key, key) == 0) {
            return 1;
        }

        index++;
        index = index % TABLE_SIZE; // allows for looping around searching for the next available spot
        pair = ht->table[index];

        if(index == starting_index) {
            return 0;
        }
    }

    // if their is a gap in the values while probing, the value must not exist

    return 0;

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

void visualize(HashTable* ht, int seq) {
    for(int i = 0; i < TABLE_SIZE; i++) {
        if(ht->table[i] != NULL) {
            printf("%s ", ht->table[i]->key);
            printf("%d\n", ht->table[i]->value);
            if (seq)
                visualize_sequential(ht->table[i]);
        }
    }
}
