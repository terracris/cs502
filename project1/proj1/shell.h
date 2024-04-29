#ifndef SHELL
#define SHELL

#define MAX_COMMANDS 32
#define MAX_PROMPT_LENGTH 16
#define MAX_BUFFER_SIZE 128

/* prototypes */
char** split(char* command);
char* set_prompt(char* prompt, char* phrase);
int is_background(char** commands);
void removeAmpersand(char** commands);
int command_length(char** commands);
void print_command(char** commands);
int shell();
int time_to_millis(long int seconds, long int usec);



#endif