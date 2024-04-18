#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include "proj6.h"

#define DEFAULT_BUFSIZE 1024
#define MAX_READ_SIZE 8192
#define MAX_THREADS 16
#define MAX_WORDS 8192

int four_or_greater = 0;
int max_len = 0;
int thread_count = 1; // default number is one thread!
int partial_flag = 0; // default not engaged
int partial_word_len = 0;

pthread_t threads[MAX_THREADS];
struct word words[MAX_THREADS][MAX_WORDS];
int words_per_thread[MAX_THREADS];
struct thread_args args[MAX_THREADS];

int main(int argc, char *argv[])
{
    int read_flag = 1; // default is to read
    int memory_mapped = 0;
    int read_size = DEFAULT_BUFSIZE; // default read size unless specified

    if (argc < 2 || argc > 4)
    {
        printf("Invalid number of arguments\n");
        exit(1);
    }

    if (argc >= 3)
    {
        if (strcmp(argv[2], "mmap") == 0)
        {
            printf("Using memory mapping\n");
            memory_mapped = 1;
        }
        else
        {
            read_size = atoi(argv[2]);
            validate_readsize(read_size);
        }
    }

    if (argc == 4)
    {
        thread_count = extract_number(argv[3]);
        printf("%d threads\n", thread_count);
    }

    const char *filename = argv[1];

    if (read_flag)
    {
        read_file(read_size, filename, thread_count);
    }

    if (memory_mapped)
    {
        memory_map_file(filename, thread_count);
    }

    return 0;
}

void read_file(int read_size, const char *filename, int thread_count)
{
    int fdIn, cnt, i;
    char buf[read_size];

    if ((fdIn = open(filename, O_RDONLY)) < 0)
    {
        perror("could not open file");
        exit(1);
    }

    int file_size = 0;

    while ((cnt = read(fdIn, buf, read_size)) > 0)
    {
        threadify(cnt, thread_count, buf);
        visualize(); // need to visualize my words
        combine();
        file_size += cnt;
    }

    printf("File size: %d\n", file_size);

    if (partial_word_len >= 4)
    {
        four_or_greater++;
    }

    if (partial_word_len > max_len)
    {
        printf("Maximum string length: %d\n", partial_word_len);
        printf("Strings of at least length 4: %d\n", four_or_greater);
    }
    else
    {
        printf("Maximum string length: %d\n", max_len);
        printf("Strings of at least length 4: %d\n", four_or_greater);
    }

    if (fdIn > 0)
    {
        close(fdIn);
    }
}

void *process(void *arg)
{
    struct thread_args *thread_arg = (struct thread_args *)arg; // convert to thread_args pointer
    int thread_id = thread_arg->id;
    int start = thread_arg->start;
    int end = thread_arg->end;
    char *buf = thread_arg->buffer;

    int curr_string_len = 0;
    int word_count = 0;

    for (int i = start; i < end; i++)
    {
        char current_byte = buf[i];

        if (isprint(current_byte) || isspace(current_byte))
        {
            // if it is a printable character, we must increase the count of bytes read
            // string lenth, and the
            curr_string_len++; // increment current string length
        }
        else
        {
            // store the word in the array

            if (curr_string_len == 0)
            {
                continue;
            }

            words[thread_id][word_count].length = curr_string_len;
            words[thread_id][word_count].is_partial = 0;
            curr_string_len = 0;
            word_count++;
        }
    }

    if (curr_string_len > 0)
    {
        // we have a partial word
        words[thread_id][word_count].length = curr_string_len;
        words[thread_id][word_count].is_partial = 1;
        word_count++;
    }

    words_per_thread[thread_id] = word_count;
}

// void threadify(int bytes_read, int thread_count, char *buf)
// {
//     // now make the threads

//     // define the chunk size by the number of bytes read
//     int chunk_size = bytes_read / thread_count; // even if its one thread reading everything
//     int remainder = bytes_read - (thread_count * chunk_size);

//     int start = 0;
//     int end = chunk_size + remainder;

//     for (int i = 0; i < thread_count; i++)
//     {
//         args[i].id = i;
//         args[i].start = start;
//         args[i].end = end;
//         args[i].buffer = buf;
//         if (pthread_create(&threads[i], NULL, (void *)process, (void *)&args[i]) != 0)
//         {
//             perror("could not create threads\n");
//             exit(1);
//         }

//         start = end;
//         end += chunk_size;
//     }

//     // then wait for all threads to terminate
//     for (int i = 0; i < thread_count; i++)
//     {
//         pthread_join(threads[i], NULL);
//     }
// }

void threadify(int bytes_read, int thread_count, char *buf)
{
    int chunk_size = (bytes_read + thread_count - 1) / thread_count; // Round up division
    int start = 0;
    int end = 0;

    for (int i = 0; i < thread_count; i++)
    {
        start = end;
        end = start + chunk_size;
        if (end > bytes_read)
        {
            end = bytes_read;
        }

        args[i].id = i;
        args[i].start = start;
        args[i].end = end;
        args[i].buffer = buf;

        if (pthread_create(&threads[i], NULL, process, (void *)&args[i]) != 0)
        {
            perror("could not create threads\n");
            exit(1);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

void combine()
{
    // look through all the words
    for (int i = 0; i < thread_count; i++)
    {
        for (int j = 0; j < words_per_thread[i]; j++)
        {
            struct word curr_word = words[i][j];
            int word_len;

            word_len = curr_word.length; // DEFAULT CASE

            // handle partials first
            if (partial_flag && !curr_word.is_partial)
            {
                word_len = partial_word_len + curr_word.length;
                partial_word_len = 0; // reset
                partial_flag = 0;     // reset
            }

            if (partial_flag && curr_word.is_partial)
            {
                // chain word length to account for long strings
                partial_word_len += curr_word.length; // add the word length to the total partial
                break;                                // found a partial, don't need to check anything else
            }

            if (curr_word.is_partial && !partial_flag)
            {
                // retain the information and lets look at the next set of words from
                // the next thread

                partial_flag = 1;
                partial_word_len = curr_word.length;
                // printf("here!!\n");
                break; // no need to keep looking!
            }

            if (word_len >= 4)
            {
                four_or_greater++;
            }

            if (word_len > max_len)
            {
                max_len = word_len;
            }
        }
    }
}

void memory_map_file(const char *filename, int thread_count)
{
    // memory mapped file
    int fd;
    char *pchFile;
    struct stat sb;

    if ((fd = open(filename, O_RDONLY)) < 0)
    {
        perror("could not open file");
        exit(1);
    }

    if (fstat(fd, &sb) < 0)
    {
        perror("coul not stat file to obtain its size\n");
        exit(1);
    }

    if ((pchFile = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (char *)-1)
    {
        perror("could not mmap file");
        exit(1);
    }

    // now that we have opened the file, we are all set to read from it
    int file_size = sb.st_size;
    int strings_of_size_4_or_greater = 0;
    int max_string_len = 0;
    int curr_string_len = 0;

    for (int i = 0; i < file_size; i++)
    {
        char current_byte = pchFile[i];
        if (isprint(current_byte) || isspace(current_byte))
        {
            // if it is a printable character, we must increase the count of bytes read
            // string lenth, and the
            curr_string_len++; // increment current string length
            // printf("%c", current_byte);
        }
        else
        {
            if (curr_string_len >= 4)
            {
                strings_of_size_4_or_greater++;
            }

            if (curr_string_len > max_string_len)
            {
                max_string_len = curr_string_len;
            }

            curr_string_len = 0;
        }
    }

    // printf("curr string len: %d\n", curr_string_len) ;

    if (curr_string_len >= 4)
    {
        strings_of_size_4_or_greater++;
    }

    if (curr_string_len > max_string_len)
    {
        max_string_len = curr_string_len;
    }

    printf("File size: %d\n", file_size);
    printf("Strings of at least length 4: %d\n", strings_of_size_4_or_greater);
    printf("Maximum string length: %d\n", max_string_len);

    if (fd > 0)
        close(fd);
}

void visualize()
{

    int total_words_seen = 0;
    for (int i = 0; i < thread_count; i++)
    {
        for (int j = 0; j < words_per_thread[i]; j++)
        {
            struct word curr_word = words[i][j];
            int word_len;

            word_len = curr_word.length; // DEFAULT CASE
            int partial = curr_word.is_partial;
            printf("thread %d. word #%d. length: %d, partial: %d\n", i, j, word_len, partial);
            if(word_len >= 4) {
                total_words_seen++;
            }
        }
    }

    printf("total words 4 or greater seen: %d\n", total_words_seen);
}

void validate_readsize(int read_size)
{
    if (read_size > MAX_READ_SIZE)
    {
        printf("read size is too large.\n");
        exit(1);
    }
}

int extract_number(const char *str)
{
    int num = 0;
    const char *ptr = str;

    // Skip non-digit characters
    while (*ptr && !isdigit(*ptr))
    {
        ptr++;
    }

    // Convert digits to integer
    while (*ptr && isdigit(*ptr))
    {
        num = num * 10 + (*ptr - '0');
        ptr++;
    }

    return num;
}
