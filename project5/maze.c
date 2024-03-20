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
int visitor_count[MAXROOMS]; /* array to keep track if the total number of visitors for each room. */
sem_t room_semaphore[MAXROOMS]; /* array of semaphores for each room. each room can have different semaphore counts*/
sem_t logbook_room_semaphore[MAXROOMS]; /* semaphore to control logbook accesss*/
time_t start_time;
int algorithm_flag;
int total_rooms;
int num_of_rats;
pthread_t rats[MAXRATS];  /* array of rat thread ids */
int total_time = 0;

/* prototypes */
void * Rat(void * arg);
void EnterRoom(int iRat, int iRoom);
void LeaveRoom(int iRat, int iRoom, int tEnter);

/* 
1. read info. [done]
2. create rooms and configs [done]
3. create semaphores for rooms [done]
4. create semaphores for logbook [done]
5. create the rat threads. [done]
6. wait for threads to terminate.  [done]
7. clean up semaphores [done]

TODO: fix main thread such that it prints out statistics
*/

int main(int argc, char** argv) {

    if (argc != 3) {
        printf("Invalid number of inputs\n");
        exit(1);
    }

    /* validate input */
    num_of_rats = atoi(argv[1]); /* store number of rats */
    const char * algorithm = argv[2];        /* store the algorithm to use */
    
    if (strcmp(algorithm, "i") == 0) {
        algorithm_flag = 0;  /* inorder traversal */
        printf("you are using inorder traversal\n");
    } else if(strcmp(algorithm, "d") == 0) {
        algorithm_flag = 1;  /* distributed traversal */
        printf("you are using distributed traversal\n");
    } else {
        perror("Invalid algorithm passed in.\n");
        exit(1);
    }

    if (num_of_rats > MAXRATS) {
        perror("You are trying to create too many rats.\n");
        exit(1);
    }

    printf("creating %d rats\n", num_of_rats);

    /* -------------------------------------------------------- */
    
    /* create rooms and config */
   
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
    int total_delay_time = 0;

    // Read and print the contents --> very happy no segmentation faults!
    while (fgets(line, sizeof(line), file) != NULL) {
        if(room_number >= MAXROOMS) {
            perror("Too many rooms!\n");
            exit(1);
        }
        
        /* Split each line into duration and capacity */
        int capacity = atoi(strtok(line, delim));
        int duration = atoi(strtok(NULL, delim));
        total_delay_time += duration;
        printf("room %d: capacity = %d, duration = %d\n", room_number, capacity, duration);

        /* here is where we can create our semaphores */
        room_config[room_number] = duration;

        int success = sem_init(&room_semaphore[room_number], 0, capacity);
        int log_success = sem_init(&logbook_room_semaphore[room_number], 0, 1);
        
        if (success != 0 || log_success != 0) {
            perror("could not make semaphore.\n");
            exit(1);
        }

        room_number++;
    }

    total_rooms = room_number; /* stores the total number of rooms */

    printf("total rooms: %d\n", total_rooms);
    // Close the file
    fclose(file);
    
    /* Create rats */
    for(long i = 0; i < num_of_rats; i++) {
        /* create rat threads */
        int success = pthread_create(&rats[i], NULL, Rat, (void * ) i) ;

        if (success != 0) {
            printf("Could not create thread\n");
            exit(1);
        }
    }
    

    /* store the start time */
   time(&start_time);

   int thread_number = 0;

   while(thread_number < num_of_rats) {
    /* wait for all threads to finish */
    pthread_join(rats[thread_number], NULL);
    thread_number++;
   }

   for(int i = 0; i < total_rooms; i++) {
    int success = sem_destroy(&room_semaphore[room_number]);
    int log_success = sem_destroy(&logbook_room_semaphore[room_number]);
        
    if (success != 0 || log_success != 0) {
            perror("could not make semaphore.\n");
            exit(1);
    }

   }

   int ideal_time = total_delay_time * num_of_rats;
   /* print total time and ideal time statistics */
   printf("Total traversal time: %d seconds, compared to ideal time: %d seconds.\n", total_time, ideal_time);

   /* print log book */


    return 0;
}


void * Rat(void * arg) {
    int rat_id = (long) arg;
    int starting_room;

    printf("Rat %d running!\n", rat_id);

    if (algorithm_flag == 0) {
        /* inorder traversal */
        starting_room = 0;
    } else {
        /* distributed traversal */
        starting_room = rat_id % total_rooms; /* starting room number based on rat id*/
    }

    int rooms_visited = 0; // at first you have not visited any rooms
    int room_to_visit = starting_room;
    int last_room_visited;

    while(rooms_visited < total_rooms) {

        // start time defines how much time for our system
        int current_time = (int) difftime(time(NULL), start_time); /* time relative to start */
        EnterRoom(rat_id, room_to_visit);
        LeaveRoom(rat_id, room_to_visit, current_time);
        last_room_visited = room_to_visit;
        room_to_visit++;
        room_to_visit %= total_rooms;  /* the rooms loop, so it is important to account for that */
        rooms_visited++;
        
    }

    printf("Rat %d exited maze\n", rat_id);

    /* to find final time, check the time it took to leave the last room the rat visited. */
    int completion_time = get_completion_time(last_room_visited, rat_id);
    total_time += completion_time;

    // once all rooms have been visited, then we are done and we print out our data.
    printf("Rat %d completed maze in %d seconds.\n", rat_id, completion_time);
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
   
   // activate semaphore for room
   sem_wait(&logbook_room_semaphore[iRoom]);

   struct vbentry log;
   log.iRat = iRat;
   log.tEntry = tEnter;
   log.tDep = (int) difftime(time(NULL), start_time);

   int entry_number = visitor_count[iRoom]; /* get the number of visitors for a given room */

   RoomVB[iRoom][entry_number] = log;
   visitor_count[iRoom]++; /* increase the visitor count */
   sem_post(&room_semaphore[iRoom]); /* increase semaphore for room */
   sem_post(&logbook_room_semaphore[iRoom]);
}

int get_completion_time(int last_room_visited, int rat_id) {

    for(int i = 0; i < num_of_rats; i++) {
        if(RoomVB[last_room_visited][i].iRat == rat_id) {
            return RoomVB[last_room_visited][i].tDep;
        }
    }
}