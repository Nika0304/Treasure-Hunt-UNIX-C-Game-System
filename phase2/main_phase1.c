#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include "treasure_manager.h"

int main(int argc, char* argv[]) 
{
    // Check for valid argument count first
    if (argc < 3 || argc > 4) 
    {
        const char *msg = "Optiunile valide sunt:\n--add huntId\n--list huntId\n--view huntId treasureId\n--remove_treasure huntId treasureId\n--remove_hunt huntId\n";
        write(1, msg, strlen(msg));
    }

    // Handle valid commands
    if (argc == 3) 
    {
        if (strcmp(argv[1], "--add") == 0) 
        {
            add_treasure(argv[2]);
        } 
        else if (strcmp(argv[1], "--list") == 0) 
        {
            list(argv[2]);
        } 
        else if (strcmp(argv[1], "--remove_hunt") == 0) 
        {
            remove_hunt(argv[2]);
        } 
        else 
        {
            const char *msg = "Optiunile valide sunt:\n--add huntId\n--list huntId\n--view huntId treasureId\n--remove_treasure huntId treasureId\n--remove_hunt huntId\n";
            write(1, msg, strlen(msg));
        }
    } 
    else if (argc == 4) 
    {
        if (strcmp(argv[1], "--view") == 0) 
        {
            view(argv[2], atoi(argv[3]));
        } 
        else if (strcmp(argv[1], "--remove_treasure") == 0) 
        {
            remove_treasure(argv[2], atoi(argv[3]));
        } 
        else 
        {
            const char *msg = "Optiunile valide sunt:\n--add huntId\n--list huntId\n--view huntId treasureId\n--remove_treasure huntId treasureId\n--remove_hunt huntId\n";
            write(1, msg, strlen(msg));
        }
    }

    return 0;
}