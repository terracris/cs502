#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <cstring>
#include <sys/resource.h>
#include <sys/time.h>
#include <math.h>
#include <sstream>
#include <vector>

using namespace std;

extern char **environ;
int time_to_millis(long int tv_sec, long int tv_usec);
int handle_command(string command);
int shell();
vector<char *> split(string command);

/*

    Program takes in two arguments:
    1. linux command to run
    2. file containing text.

    for instance ./doit wc foo.txt runs the word count command with argument foo.txt
    which is assumed to exist in the same directory. 

*/
int main(int argc, char **argv) {
    
  if (argc > 1) {
    // if we have more than one argument, we know we actually have a command
        // handle_command(argv);
        cout << "whoops!" << endl;
    }
    else {
        shell();
    }
    

}

int shell() {
    string command;
    cout << "==> ";

    while(getline(cin, command)) {
        if (command == "exit") 
            break;

        if(command.empty()) {
            continue;
        }

        handle_command(command);
        cout << "==> ";
    }

    cout << "got the end of file or exit" << endl;

   exit(0); 
}

vector<char *> split(string command) {
    // create an input string stream from the commands
    istringstream iss(command);

    // vector to store individual words
    vector<char *> words;

    // Read words from the input string stream
    char * word;

    // the >> operator automatically splits strings by spaces 
    while(iss >> word) {
        char *p = (char *) malloc(sizeof(char) * (strlen(word) + 1));
        strcpy(p, word);
        words.push_back(p);
    }

    return words;
}

int handle_command(string command) { 

    vector<char *> commands = split(command);

    cout << "command: " << commands[0] << "\n";
    cout << "input: " << commands[1] << endl;

    char *argvNew[38]; // +1 for NULL
    int pid;
    struct timeval start, end;

    // get the current time before the operation
    gettimeofday(&start, NULL);

    if((pid = fork()) < 0) {
        cerr << "Fork error\n";
        exit(1);
    }
    else if(pid == 0) {
        // child process
        int i = 0;
        for(char * w : commands) {
            cout << "word: " << w << endl;
            argvNew[i] = w;
            i++;
        }

        argvNew[i] = NULL;

        cout << "execvp item 1: " << argvNew[0] << endl;
        cout << "execvp item 2: " << argvNew[1] << endl;
        

        if(execvp(argvNew[0], argvNew) < 0) {
            cerr << "Execute error\n";
            exit(1);
        }

        exit(0);
    
    }
    else {
        // parent
        wait(0);
        gettimeofday(&end, NULL);

        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        int elapsed = time_to_millis(seconds, microseconds);


        struct rusage usage;
        if (getrusage(RUSAGE_CHILDREN, &usage) == 0) {
            cout << "User CPU time (milliseconds): " << time_to_millis(usage.ru_utime.tv_sec, usage.ru_utime.tv_usec) << "\n";
            cout << "System CPU time (milliseconds): " << time_to_millis(usage.ru_stime.tv_sec, usage.ru_stime.tv_usec) << "\n";
            cout << "elapsed time (milliseconds): " << elapsed << "\n";
            cout << "# times process was preempted involuntarily: " << usage.ru_nivcsw << "\n";
            cout << "# times process was preempted voluntarily: " << usage.ru_nvcsw << "\n";
            cout << "# of major page fault: " << usage.ru_majflt << "\n";
            cout << "# of minor page fault: " << usage.ru_minflt <<  "\n";
            cout << "max resident set sized used: " << usage.ru_maxrss << "\n";
        } else {
            cerr << "Could not get usage statistics\n";
            exit(1);
        }
    }

    return 0;
}

/**
 * @brief calculates the number of milliseconds elapsed based ont he timeval
 * 
 * @param tv_sec number of whole seconds of elapsed time
 * @param tv_usec number of microseconds elapsed
 * @return int number of milliseconds elapsed
 */
int time_to_millis(long int seconds, long int usec) {
    // cout << "\nSeconds: " << tv_sec << "\n";
    // cout << "Microseconds: " << tv_usec << "\n";
    int result = (seconds * 1000) + ceil(float(usec) / 1000.0); // implicit conversion to milliseconds

    return result;
}