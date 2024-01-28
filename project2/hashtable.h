#ifndef HASHTABLE
#define HASHTABLE

#define TABLE_SIZE 200

typedef struct Sequential {
    char* call;
    unsigned int count;
    struct Sequential* next;
} Sequential;

// Define a structure for key-value pairs
typedef struct {
    char* key;
    unsigned int value; // number of times a sysytem call appears
    Sequential* sequential;
} KeyValuePair;

// a hashtable is just a collection of key-value pairs
// define a structure for the hashtable
typedef struct {
    KeyValuePair* table[TABLE_SIZE]; // this says any intance of a HashTable has an array of key value pairs
} HashTable;

unsigned int hash(const char* key);
void initHashTable(HashTable* ht);
void insert(HashTable* ht, const char* key, const char* subsequent);
int get(HashTable* ht, const char* key);
int contains(HashTable* ht, const char* key);
void freeHashTable(HashTable* ht);
void visualize(HashTable* ht);
Sequential * create_sequential(const char* call);
void add_sequential(KeyValuePair* current, const char* call);
void visualize_sequential(KeyValuePair* current);
void free_subsequent_calls(KeyValuePair* ht);


#endif