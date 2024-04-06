#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#define DEFAULT_BUFSIZE 1024
#define MAX_READ_SIZE 8192

void validate_readsize(int);
int main(int argc, char *argv[])
{
    int fdIn, cnt, i;
    int read_flag = 0;
    int status = 0;
    int read_size;

    switch (argc)
    {
    case 1:
        printf("invalid number of arguments\n");
        exit(1);

    case 2:
        // default using read
        read_flag = 1;
        read_size = DEFAULT_BUFSIZE;
        printf("using default\n");
        break;

    case 3:
        // 3 input arguments, need to check if we are swapping the default size or if using mmap
        // status flag is 0 if change in read size, 1 if memory mapping
        if(strcmp(argv[2], "mmap") == 0) {
            printf("using memory mapping\n");
            status = 1;
        } else {
            read_size = atoi(argv[2]);
            validate_readsize(read_size);
            read_flag = 1;
        }
        break;
    }

    if (read_flag)
    {
        char buf[read_size];
        if ((fdIn = open(argv[1], O_RDONLY)) < 0)
        {
            fprintf(stderr, "could not open file\n");
            exit(1);
        }

        printf("%d\n", fdIn);

        // now that we have opened the file, we are all set to read from it
        int file_size = 0;
        int strings_of_size_4_or_greater = 0;
        int max_string_len = 0;
        int curr_string_len = 0;

        while ((cnt = read(fdIn, buf, read_size)) > 0)
        {
            // now that we have read, we need to look at every byte that we read
            // we need to determine if it is a printable character

            for (int i = 0; i < cnt; i++)
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

            file_size += cnt;
        }

        // final check after exiting the while loop
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

        if (fdIn > 0)
            close(fdIn);
    }

    if(status) {
        // memory mapped file
        int fd;
        char *pchFile;
        struct stat sb;

        if((fd = open(argv[1], O_RDONLY)) < 0) {
            perror("could not open file");
            exit(1);
        }

        if(fstat(fd, &sb) < 0) {
            perror("coul not stat file to obtain its size\n");
            exit(1);
        }

        if((pchFile = (char *) mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (char *) -1) {
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
                    printf("%c", current_byte);
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

            printf("curr string len: %d\n", curr_string_len) ;

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

        if (fdIn > 0)
            close(fdIn);
    }
}


void validate_readsize(int read_size) {
    if (read_size > MAX_READ_SIZE) {
        printf("read size is too large.\n");
        exit(1);
    }
}
