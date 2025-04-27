#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include "treasure_manager.h"

#define MAX 256
int monitor_pid=0;
/*
void commandFile(char* msg)
{
    int commandFile=open("commands.txt", O_APPEND | O_CREAT | O_RDWR, 0777);
    if (commandFile == -1)
    {
        perror("eroare la deschiderea fisierului commandFile.txt");
        exit(-1);
    }
    write(commandFile, msg, strlen(msg));
    close(commandFile);
}
*/

void list_hunts(int signal)
{
    char msg[256] = {0};

    char cwd[1024];
    if (getcwd(cwd,sizeof(cwd)) == NULL)
    {
        perror("eroare la directorul curent");
        exit(-1);
    }

    if (signal == SIGUSR1)
    {
        strcpy(msg,"List the hunts and the toatal number of treasures im each. ");
        AllHunts(cwd);
    }
}

void AllTreasures(const char* path)
{

}
/*
void AllHunts(const char* dir)
{
    DIR *d=opendir(dir);

    if (d == NULL)
    {
        perror("eroare la deschiderea directorului");
        exit(-1);
    }

    struct dirent *dname;
    char path_hunt[MAX];
    char path_tr[MAX];

    int count;

    while((dname=readdir(d)) != NULL)
    {
        if(strcmp(dname->d_name, ".") == 0 || strcmp(dname->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(path_hunt, sizeof(path_hunt), "%s/%s",dir,dname->d_name);
        struct stat st;

        if (dname->d_type == DT_DIR)
        {
            printf("Hunt name: %s -- ",dname->d_name);
            count=0;
            AllHunts(path_hunt);
        }

        if (lstat(path_hunt, &st) == -1) {
            perror("eroare la stat");
            continue;
        }
        
        else if (S_ISREG(st.st_mode))
        {
            Treasure t;
            snprintf(path_tr,sizeof(path_tr),"%s/treasures%s.dat",path_hunt,dname->d_name);

            if (strcmp(path_hunt,path_tr) == 0)
            {
                int tr=open(path_tr,O_RDONLY,0777);
                while(read(tr,&t,sizeof(t))==sizeof(t))
                {
                    count++;
                }
                close(tr);
                printf("%d treasure files\n",count);
            }
        }

    }
    closedir(d);
}
*/
void AllHunts(const char* dir)
{
    DIR *d = opendir(dir);
    if (d == NULL)
    {
        perror("eroare la deschiderea directorului");
        exit(-1);
    }

    struct dirent *dname;
    char path_hunt[MAX];
    int count;

    while ((dname = readdir(d)) != NULL)
    {
        if (strcmp(dname->d_name, ".") == 0 || strcmp(dname->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(path_hunt, sizeof(path_hunt), "%s/%s", dir, dname->d_name);

        struct stat st;
        if (lstat(path_hunt, &st) == -1)
        {
            perror("eroare la stat");
            continue;
        }

        if (dname->d_type == DT_DIR)
        {
            printf("Hunt name: %s -- ", dname->d_name);
            count = 0;

            // Verificăm dacă folderul conține fișiere de tipul "treasures.dat"
            char path_tr[MAX];
            snprintf(path_tr, sizeof(path_tr), "%s/treasures%s.dat", path_hunt,dname->d_name);

            // Verificăm existența fișierului treasures.dat
            if (access(path_tr, F_OK) != -1) 
            {
                Treasure t;
                int tr = open(path_tr, O_RDONLY, 0777);
                while (read(tr, &t, sizeof(t)) == sizeof(t))
                {
                    count++;
                }
                close(tr);
                printf("%d treasure files\n", count);
            }
            else
            {
                printf("No treasure files found.\n");
            }
        }
    }
    closedir(d);
}

void start_monitor()
{
    if (monitor_pid != 0)
    {
        printf("Monitorul deja ruleaza\n");
        return;
    }

    monitor_pid = fork();

    if (monitor_pid < 0)
    {
        perror("eroare la fork");
        exit(-1);
    }

    if (monitor_pid == 0)
    {
        struct sigaction sa1;
        sa1.sa_handler = list_hunts;
        sigemptyset(&sa1.sa_mask);
        sa1.sa_flags = 0;
        sigaction(SIGUSR1, &sa1, NULL);

        while(1)
        {
            pause();
        }
    }
    else
    {
        sleep(1);
        printf("At PID %d, start_monitor command was given\n", monitor_pid);
        sleep(2);
    }
}

int main()
{
    int choice;
    while(1)
    {
        printf("Welcome to the Treasure Manager!\n");
        printf("1. Start Monitor\n");
        printf("2. List Hunts\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                start_monitor();
                break;
            case 2:
                // Trimite semnalul SIGUSR1 pentru a lista vânătoarele
                if (monitor_pid != 0)
                {
                    kill(monitor_pid, SIGUSR1);
                }
                else
                {
                    printf("Monitorul nu a fost pornit. Trebuie sa rulezi Start Monitor mai intai.\n");
                }
                break;
            case 3:
                printf("Exiting...\n");
                if (monitor_pid != 0)
                {
                    kill(monitor_pid, SIGKILL);  // Oprim procesul monitor
                }
                exit(0);
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}