#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include "treasure_manager.h"


int main() 
{
    // Set up signal handler for child process termination
    struct sigaction sa_chld;
    sa_chld.sa_handler = handler_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = 0;   
    if (sigaction(SIGCHLD, &sa_chld, NULL) < 0) 
    {
        perror("sigaction failed");
        exit(1);
    } 

    char choice[256];

    while (1) 
    {
        printf("\n--- Treasure Manager ---\n");
        printf("start_monitor\nlist_hunts\nlist_treasures [huntId]\nview_treasure [huntId] [id]\ncalculate_score\nstop_monitor\nexit\n> ");
        fflush(stdout);

        fgets(choice, sizeof(choice), stdin);

        choice[strcspn(choice, "\n")] = '\0'; // Remove newline character

        if (strcmp(choice, "start_monitor") == 0) 
        {
            start_monitor();
        } 
        else if (strcmp(choice, "list_hunts") == 0) 
        {

            check_monitor_status();

            if (monitor_running && monitor_stopping == 0) 
                commandFile(choice);

        } 
        else if (strncmp(choice, "list_treasures", 14) == 0) 
        {

            check_monitor_status();

            if (monitor_running && monitor_stopping == 0) 
                commandFile(choice);

        } 
        else if (strncmp(choice, "view_treasure", 13) == 0) 
        {

            check_monitor_status();

            if (monitor_running && monitor_stopping == 0) 
                commandFile(choice);

        }
        else if (strcmp(choice, "calculate_score") == 0) 
        {
            check_monitor_status();

            if (monitor_running && monitor_stopping == 0) 
                commandFile(choice);
        }   


        else if (strcmp(choice, "stop_monitor") == 0)
        {
                stop_monitor();
        }
        else if (strcmp(choice, "exit") == 0) 
        {

            if (monitor_running) 
                printf("Monitor is still running\n");
            else
                break;

        }
        else 
            printf("Unknown command: %s\n", choice);
    }
    return 0;
}
