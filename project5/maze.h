#ifndef MAZE
#define MAZE

#define MAXRATS 5
#define MAXROOMS 8

struct vbentry {
    int iRat; /* rat identifier */
    int tEntry; /* time of entry into room */
    int tDep;  /* time of departure from room */
};


int get_completion_time(int last_room_visited, int rat_id);

#endif