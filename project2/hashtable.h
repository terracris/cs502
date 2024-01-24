#ifndef HASHTABLE
#define HASHTABLE

#define TABLE_SIZE 200

// Define a structure for key-value pairs
typedef struct {
    char* key;
    unsigned int value; // number of times a sysytem call appears
} KeyValuePair;

// a hashtable is just a collection of key-value pairs
// define a structure for the hashtable
typedef struct {
    KeyValuePair* table[TABLE_SIZE]; // this says any intance of a HashTable has an array of key value pairs
} HashTable;

unsigned int hash(const char* key);
void initHashTable(HashTable* ht);
void insert(HashTable* ht, const char* key, int value);
int get(HashTable* ht, const char* key);
int contains(HashTable* ht, const char* key);
void freeHashTable(HashTable* ht);
void visualize(HashTable* ht);


#endif