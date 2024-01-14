/* myexec.C */

#include <iostream>
using namespace std;
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <cstring>

extern char **environ;		/* environment info */

int main(int argc, char **argv)
     /* argc -- number of arguments */
     /* argv -- an array of strings */
{
    char *argvNew[5];
    int pid;

    if ((pid = fork()) < 0) {
	cerr << "Fork error\n";
        exit(1);
    }
    else if (pid == 0) {
        /* child process */

        argvNew[0] = "ls";
        argvNew[1] = "-l";
        argvNew[2] = NULL;
        if (execvp(argvNew[0], argvNew) < 0) {
	    cerr << "Execve error\n";
            exit(1);
        }
    }
    else {
        /* parent */
	wait(0);		/* wait for the child to finish */
    }
}
