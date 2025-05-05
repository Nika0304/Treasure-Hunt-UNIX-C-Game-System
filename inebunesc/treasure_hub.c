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


#define MAX 256

int monitor_pid = -1;
int monitor_stopping = 0;
int monitor_running = 0;


// Function to check the current status of the monitor
void check_monitor_status()
{
    if (monitor_running == 0) 
    {
        printf("Monitor not running\n");
    }
    else if (monitor_stopping == 1) 
    {
        printf("Monitor is stopping\n");
    }
}

// Writes a command to the "command.txt" file and sends SIGUSR1 to the monitor.
void commandFile(char* msg)
{
    int fd = openFileWithCheck("command.txt", O_WRONLY | O_CREAT | O_TRUNC, "Error writing to command.txt");
   
    write(fd, msg, strlen(msg));
    closeFileWithCheck(fd, "Error closing command.txt");

    if (monitor_pid > 0)
        kill(monitor_pid, SIGUSR1); // Notify the monitor to read the command

    usleep(2000000); // Wait 2 seconds
}

// Returns the number of treasures in the specified hunt directory.
int TresuresCount(const char* huntId)
{
    char path[256];
    getTreasureFilePath(huntId, path);

    if (!DirectorulExista(huntId)) 
    {
        perror("This directory does not exist or is not a directory");
        return 0; 
    }

    int TresureFile = openFileWithCheck(path, O_RDONLY, "Error opening treasure file in TreasureCount");
    
    Treasure t;
    ssize_t bytes_read;
    int count = 0;

    while ((bytes_read = read(TresureFile, &t, sizeof(Treasure))) == sizeof(Treasure))
    {
        count++;
    }

    // Check for file corruption or read errors
    if (bytes_read > 0 && bytes_read != sizeof(Treasure))
    {
        fprintf(stderr, "Corrupted treasure file '%s': read %zd bytes (expected %zu).\n", path, bytes_read, sizeof(Treasure));
        closeFileWithCheck(TresureFile, "Error closing treasure file");
        exit(EXIT_FAILURE);
    }
    else if (bytes_read < 0)
    {
        perror("Read error");
        closeFileWithCheck(TresureFile, "Error closing treasure file");
        exit(EXIT_FAILURE);
    }

    closeFileWithCheck(TresureFile, "Error closing treasure file in TreasureCount");
    return count;
}

//Lists all hunt directories in the current working directory.
void AllHunts()
{
    DIR *d = opendir(".");
    if (d == NULL)
    {
        perror("Error opening current directory");
        exit(-1);
    }

    struct dirent *dname;
    int gasit = 0;

    // Loop through each entry in the directory
    while ((dname = readdir(d)) != NULL)
    {
        if (strcmp(dname->d_name, ".") == 0 || strcmp(dname->d_name, "..") == 0)
        {
            continue; // Skip "." and ".." entries
        }
        
        if (dname->d_type == DT_DIR) // daca e director
        {
            gasit=1;
            printf("Hunt name: %s -- Number of Tresures: %d\n", dname->d_name, TresuresCount(dname->d_name));
        }
    }

    if (gasit == 0)
    {
        printf("This directory does not exist or it contains 0 treasures\n");
    }

    closedir(d);
}

//Signal handler for SIGUSR1 — reads command from file and executes it.
void handle_signal1(int sig)
{
    //Wait briefly to allow commandFile() to finish writing
    usleep(100000);  // 100ms delay

    int fd = openFileWithCheck("command.txt", O_RDONLY, "Error opening command.txt in handle_signal1");

    char buffer[256];
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes <= 0) 
    {
        perror("Error reading from command.txt");
        closeFileWithCheck(fd, "Error closing file");
        return;
    }

    buffer[bytes] = '\0';  // null-terminate corect

    closeFileWithCheck(fd, "Error closing command.txt in handle_signal1");

    //printf("Citite %ld bytes din command.txt\n", bytes);
    char *aux = strtok(buffer, " \n");
    if (aux == NULL)
        return;

    if(strcmp(aux, "list_hunts") == 0)
    {
        AllHunts();
    }
    else if(strcmp(aux, "view_treasure") == 0)
    {

        char *huntID = strtok(NULL, " \n");
        char *id = strtok(NULL, " \n");
        
        if (huntID && id)
            view(huntID, strtol(id, NULL, 10));
        else
            printf("Invalid arguments for 'view_treasure'.\n");
    
    }
    else if(strcmp(aux, "list_treasures") == 0)
    {

        char *huntID=strtok(NULL," \n");

        if (huntID)
            list(huntID);
        else
            printf("Missing hunt ID for 'list_treasures'.\n");

    }
    else
        printf("Unknown command: %s\n", aux);

}

//Signal handler for SIGUSR2 — stop the monitor.
void handleSignalStop(int sig)
{
    printf("Monitor stopping\n");
    usleep(2000000); //2 seconds 
    exit(0); // Terminate the process
}

void handler_sigchld(int sig)
{
   int status;
   // asteptam terminarea procesului monitor
   waitpid(monitor_pid, &status, 0);
   printf("\nMonitor ended with code %d\n", WEXITSTATUS(status));
   
   monitor_running = 0;
   monitor_pid = -1;
   monitor_stopping = 0;
   
}


//Starts the monitor as a child process that handles signals
void start_monitor()
{
    if (monitor_running)
    {
        printf("Monitor is already running.\n");
        return;
    }

    // Create a new process
    int pid = fork();
    if (pid < 0)
    {
        perror("Error during fork");
        exit(-1);
    }

    else if (pid == 0)
    {
        // Child process (monitor)
        struct sigaction sa1;
        sa1.sa_handler = handle_signal1;  // Set signal handler for SIGUSR1
        sigemptyset(&sa1.sa_mask);
        sa1.sa_flags = 0;

        if (sigaction(SIGUSR1, &sa1, NULL) < 0) 
        {
            perror("sigaction failed");
            exit(1);
        } 
        
        struct sigaction sa2;
        sa2.sa_handler = handleSignalStop; // Set signal handler for SIGUSR2
        sigemptyset(&sa2.sa_mask);
        sa2.sa_flags = 0;

        if (sigaction(SIGUSR2, &sa2, NULL) < 0) 
        {
            perror("sigaction failed");
            exit(1);
        }

        // Wait for signals
        while(1)
        {
            pause(); // Pause until a signal is received
        }

        exit(0);
    }
    else
    {
        // Parent process
        monitor_pid = pid;
        monitor_running = 1;
        monitor_stopping = 0;
        sleep(1);
        printf("Monitor started (PID: %d)\n", monitor_pid);
    }
}

//Sends SIGUSR2 to stop the monitor process.
void stop_monitor() 
{
    if (monitor_running == 0) 
    {
        printf("Monitor is not running\n");
    } 
    else if (monitor_stopping == 1) 
    {
        printf("Monitor is already stopping\n");
    } 
    else 
    {
        monitor_stopping = 1;
        kill(monitor_pid, SIGUSR2);
        usleep(10000);  
    }
}
