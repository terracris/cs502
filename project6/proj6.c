#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#define BUFSIZE 1024

int main(int argc, char *argv[])
{
    char buf[BUFSIZE];
    int fdIn, cnt, i;
    int default_flag = 0;
    int status = 0;

    switch (argc)
    {
    case 1:
        printf("invalid number of arguments\n");
        exit(1);

    case 2:
        // default using read
        default_flag = 1;
        printf("using default\n");
        break;

    case 3:
        // 3 input arguments, need to check if we are swapping the default size or if using mmap
        // status flag is 0 if change in read size, 1 if memory mapping
        printf("unimplemented\n");
        exit(0);
    }

    if (default_flag)
    {
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

        while ((cnt = read(fdIn, buf, BUFSIZE)) > 0)
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
}
