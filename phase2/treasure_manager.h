#ifndef treasure_hunt_h
#define treasure_hunt_h

#define USERNAME_SIZE 32
#define CLUE_SIZE 128
#define MAX_PATH 256


typedef struct 
{
    int id;
    char username[USERNAME_SIZE];
    float latitude;
    float longitude;
    char clue[CLUE_SIZE];
    int value;
} Treasure;

//functii pentru phase 1(restul din treasure_manager.c nu le apelez in main)
void add_treasure(const char* huntId);
void list(const char *huntId);
void view(const char *huntId, int id);
void remove_treasure(const char *huntId, int id_to_remove);
void remove_hunt(const char *huntId);


//functii pentru phase2
void start_monitor();
void stop_monitor();
void AllHunts(const char* dir);
void AllTreasures(const char* path);


#endif