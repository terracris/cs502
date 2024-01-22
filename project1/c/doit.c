#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "shell.h"
#include "process.h"


int main(int argc, char **argv) {
    if (argc > 1) {

    }
    else {
        shell();
    }
    return 0;
}

int handle_input(int argc, char** argv) {

    // skip the first input

    for(int i = 1; i < argc; i++) {

    }

}

