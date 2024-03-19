#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "maze.h"

struct vbentry RoomVB[MAXROOMS][MAXRATS]; /* array of room visitor books*/
int room_config[MAXROOMS];  /* array of room duration information */
pthread_t rats[MAXRATS];  /* array of rat thread ids */
int visitor_count[MAXROOMS]; /* array to keep track if the total number of visitors for each room. */
sem_t room_semaphore[MAXROOMS]; /* array of semaphores for each room. each room can have different semaphore counts*/
sem_t logbook_room_semaphore[MAXROOMS]; /* semaphore to control logbook accesss*/
time_t start_time;
int algorithm_flag;
int total_rooms;

#define DEBUG 0

/* prototypes */
void * Rat(void * arg);

int main(int argc, char** argv) {

    if (argc != 3 && DEBUG) {
        printf("Invalid number of inputs\n");
        exit(1);
    }

    if(DEBUG) {
    /* Create rats */
    int num_of_rats = atoi(argv[1]); /* store number of rats */
    char algorithm = argv[2];        /* store the algorithm to use */
    
    if (strcmp(algorithm, "i") == 0) {
        algorithm_flag = 0;  /* inorder traversal */
    } else if(strcmp(algorithm, "d") == 0) {
        algorithm_flag = 1;  /* distributed traversal */
    } else {
        perror("Invalid algorithm passed in.\n");
        exit(1);
    }

    if (num_of_rats > MAXRATS) {
        perror("You are trying to create too many rats.\n");
        exit(1);
    }
    
    for(int i = 0; i < num_of_rats; i++) {
        /* create rat threads */
        int success = pthread_create(&rats[i], NULL, Rat, (void * ) i) ;

        if (success != 0) {
            printf("Could not create thread\n");
            exit(1);
        }
    }
    }

    /* store the start time */
    time(&start_time);

    FILE *file;
    char filename[] = "rooms.txt";
    char line[100];
    const char * delim = " ";

    // Open the file
    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return 1;
    }

    int room_number = 0;

    // Read and print the contents --> very happy no segmentation faults!
    while (fgets(line, sizeof(line), file) != NULL) {
        if(room_number >= MAXROOMS) {
            perror("Too many rooms!\n");
            exit(1);
        }
        
        /* Split each line into duration and capacity */
        int capacity = atoi(strtok(line, delim));
        int duration = atoi(strtok(NULL, delim));

        /* here is where we can create our semaphores */
        room_config[room_number] = duration;

        int success = sem_init(&room_semaphore[room_number], 0, capacity);
        
        if (success != 0) {
            perror("could not make semaphore.\n");
            exit(1);
        }

        room_number++;
    }

    total_rooms = room_number; /* stores the total number of rooms */

    // Close the file
    fclose(file);

    /* TODO
    
    1. Create rats using pthread_create() --> Each rat has its own ID [ DONE ]
    2. Read from rooms file and create the rooms --> Global variable  [ DONE ]
    3. Make data structure to contain room data (time it takes to complete and whatnot)
    4. Possible an array of room structs
    5. Create synchronization primitives (semaphores) to ensure that  [ DONE ]
       that no more rat threads are allowed into a room than the capacity of a room.
            -> mutual exclusion problem
       
    6. Write LeaveRoom()
        -> When a rat leaves a room, it must add an entry to the entry book for that
           specific room (time of entry and departure)
           
    7. Store the starting time as a variable in main and make all time relative to this.
    8. Create visitor count to keep track of the number of visitors in each room
        -> you will need to use this variable to determine the next location in the 
            VB array to enter a visitor
            
    ! Beweare that more than one rat may be trying to log information to the visitors 
      book at the same time. 
      
      Similarly semaphores could be created to control access to each room
      
    9. Wait for all rat threads to terminate, with pthread_join()
    
    ! As each rat finishes the maze, it should print its completion time in seconds
      relative to the start of the program. 
      
    ! Each rat should also add its time to a global total time variablr
    
    After all rats have terminated, the main thread should print (one line per room)
    */

    return 0;
}


void * Rat(void * arg) {
    int rat_id = (int) arg;
    int starting_room;

    if (algorithm_flag == 0) {
        /* inorder traversal */
        starting_room = 0;
    } else {
        /* distributed traversal */
        starting_room = rat_id % total_rooms; /* starting room number based on rat id*/
    }

    int rooms_visited = 0; // at first you have not visited any rooms

    while(rooms_visited < total_rooms) {

        // start time defines how much time for our system
        int current_time = time(NULL) - start_time; /* time relative to start */
        EnterRoom(rat_id, starting_room);
        LeaveRoom(rat_id, starting_room, current_time);
    }
}

/* iRat - id of rat entering the room */
/* iRoom - id of room being entered */
void EnterRoom(int iRat, int iRoom) {
    /* what does it mean to enter a room?

    1. Decrement semaphore count for that specific room.
    3. you get room duration time
    4. you call sleep for the room duration time. 
    
    */

   sem_wait(&room_semaphore[iRoom]);  /* decrement semaphore for room */
   int duration = room_config[iRoom]; /* get room duration */
   sleep(duration);                   /* sleep */

}

void LeaveRoom(int iRat, int iRoom, int tEnter) {
    /*
    1. decrement the semaphore for logging info into the log book
    2. Insert information for specific rat in the log book
    */

   /* does the data persist? I am unsure (ask during OS office hours)*/
   /* TO ASK: do I need a semaphore for logging information? is that relevant?*/
   
   // activate semaphore for room
   sem_wait(&logbook_room_semaphore[iRoom]);

   struct vbentry log;
   log.iRat = iRat;
   log.tEntry = tEnter;
   log.tDep = time(NULL) - start_time;

   int entry_number = visitor_count[iRoom]; /* get the number of visitors for a given room */

   RoomVB[iRoom][entry_number] = log;
   visitor_count[iRoom]++; /* increase the visitor count */
   sem_post(&room_semaphore[iRoom]); /* increase semaphore for room */
   sem_post(&logbook_room_semaphore[iRoom]);
}