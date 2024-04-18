#ifndef PROJ_6
#define PROJ_6

struct word {
    int length;
    int is_partial;
};

struct thread_args {
    int id;
    int start;
    int end;
    char * buffer;
};

// prototypes
void validate_readsize(int);
void combine();
int extract_number(const char *str);
void * process(void * arg);
void read_file(int read_size, const char * filename, int thread_count);
void memory_map_file(const char * filename, int thread_count);
void threadify(int bytes_read, int thread_count, char * buf);
void visualize();

#endif