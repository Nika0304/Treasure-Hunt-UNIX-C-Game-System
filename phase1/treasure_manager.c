#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#define USERNAME_SIZE 32
#define CLUE_SIZE 128
#define MAX_PATH 256

typedef struct {
    int id;
    char username[USERNAME_SIZE];
    float latitude;
    float longitude;
    char clue[CLUE_SIZE];
    int value;
} Treasure;
// Functia care verifica daca un director exista
int DirectorulExista(const char *numeDirector) {
    struct stat st;
    return (lstat(numeDirector, &st) == 0 && S_ISDIR(st.st_mode));
}

void CreareVerificareDirector(const char *huntId)
{
    //cream si verificam directorul cu numele huntId
    if(!DirectorulExista(huntId)) 
    {
        if ((mkdir(huntId,0777))==-1)
        {
            perror("Error creating directory");
            exit(EXIT_FAILURE);
        }
        else
        {
            write(1, "The directory was created successfully\n", strlen("The directory was created successfully\n"));
        }
    }
    else
    {
        write(1, "The directory already exists\n", strlen("The directory already exists\n"));
    }
}

//functia pentru creearea unei legaturi simbolice
void createSymlink(const char* huntId,const char *log_path) {
    //inca nu stiu cum se face :((; intreaba miercuri la lab

    //daca am inteles bine logged_hunt-<ID> din directorul radacina al programului va face referire la fisierul original logged_hunt din fiecare director hunt.
    //exemplu logged_hunt-hunt1 va fi o legatura simbolica care va face referire la hunt1/loggedhunt1.txt.
    char link_path[MAX_PATH];
    snprintf(link_path, sizeof(link_path), "logged_hunt-%s", huntId);
    if(access(link_path,F_OK)==-1)
    {
        if(symlink(log_path,link_path)==-1)
        {
            perror("error creating symlink");
            exit(-1);
        }
    }
}

//functia pentru fisierul de log specific fiecarui director

//de intrebat daca ajunge doar comentariu "added treasure" sau trebuie si toate detaliile despre treasure-ul adaugat
void log_operation(const char* hunt_id, const char* operation) {
    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "%s/logged%s.txt", hunt_id,hunt_id);

    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0777);
    if (fd == -1) {
        perror("Error opening the log");
        return;
    }

    struct stat st;
    if (lstat(hunt_id, &st) == -1) {
        perror("Error getting directory information");
        close(fd);
        return;
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[%s] %s\n", strtok(ctime(&st.st_ctime), "\n"), operation);
    write(fd, buffer, strlen(buffer));
    close(fd);

    createSymlink(hunt_id,log_path);
}

void list(const char *huntId)
{
    char file_path[MAX_PATH];
    snprintf(file_path, MAX_PATH, "%s/treasures%s.dat", huntId, huntId);

    if(!DirectorulExista(huntId)) 
    {
        perror("This directory does not exist or is not a directory");
        exit(-1);
    }

    //afisam numele directorului
    write(1,"HuntName: ",strlen("HuntName: "));
    write(1,huntId,strlen(huntId));

    //verificare daca exista fisierul .dat
    struct stat st;
    if (lstat(file_path, &st) != 0 || !S_ISREG(st.st_mode)) 
    {
        perror("The file does not exist or is not a regular file\n");
        exit(-1);
    }

    //afisare totalSize
    char totalSize[256];
    snprintf(totalSize,sizeof(totalSize),"\nTotal size in bytes: %lld\n",st.st_size);
    write(1,totalSize,strlen(totalSize));

    //afisare ultima modificare
    char lastModification[256];
    snprintf(lastModification,sizeof(lastModification),"Last modification : %s",ctime(&st.st_mtime));
    write(1,lastModification,strlen(lastModification));

    //afisare tresures 
    int file = open(file_path, O_RDONLY);
    if (file == -1) 
    {
        perror("Error opening the treasure file");
        exit(EXIT_FAILURE);
    }   
    Treasure *t=(Treasure*)malloc(sizeof(Treasure));
    if(t==NULL)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    lseek(file,0,SEEK_SET);
    int find=0;
    char buffer[256];
    write(1,"\nInformation about treasures\n",strlen("\nInformation about treasures\n"));
    while(read(file,t,sizeof(*t))==sizeof(*t))
    {
        find++;
        int len = snprintf(buffer, sizeof(buffer), "Treasure: %d\n", find);
        write(1, buffer, len);
       
        len = snprintf(buffer, sizeof(buffer), "ID: %d\n", t->id);
        write(1, buffer, len);

        len = snprintf(buffer, sizeof(buffer), "UserName: %s\n", t->username);
        write(1, buffer, len);

        len = snprintf(buffer, sizeof(buffer), "Latitudine: %.2f -- Longitudine: %.2f\n",  t->latitude,t->longitude);
        write(1, buffer, len);      

        len = snprintf(buffer, sizeof(buffer), "Clue: %s\n",  t->clue);
        write(1, buffer, len); 

        len = snprintf(buffer, sizeof(buffer), "Value: %d\n", t->value);
        write(1, buffer, len);
       
    
        write(1,"___________________________________________\n",strlen("___________________________________________\n"));
    }

    if(find==0)
    {
        write(1,"empty\n",strlen("empty\n"));
    }
    free(t);
    log_operation(huntId,"list hunt");
    close(file);
}


//functie pentru citirea datelor unui trasure
void add_treasure(const char *huntId)
{
    CreareVerificareDirector(huntId);

    char file_path[MAX_PATH];
    snprintf(file_path,MAX_PATH,"%s/treasures%s.dat",huntId,huntId);
    int treasure_file=open(file_path, O_WRONLY  | O_CREAT | O_APPEND, 0777);
    if (treasure_file==-1)
    {
        perror("Error opening the treasure file");
        close(treasure_file);
        return;
    }

    Treasure new_treasure;
    char buffer[32];
    write(1,"Introduce treasure date below:\n",strlen("Introduce treasure date below:\n"));

    char msg[50];
    //introducem id
    strcpy(msg,"ID:");
    write(1,msg,strlen(msg));
    read(0,buffer,sizeof(buffer));
    new_treasure.id=atoi(buffer);

    //introducem username
    strcpy(msg,"Username:");
    write(1,msg,strlen(msg));
    read(0,new_treasure.username,sizeof(new_treasure.username));
    new_treasure.username[strcspn(new_treasure.username, "\n")] = '\0';

    //introducem latitudine
    strcpy(msg,"Latitudine:");
    write(1,msg,strlen(msg));
    read(0,buffer,sizeof(buffer));
    new_treasure.latitude=atof(buffer);

    if(new_treasure.latitude>90.0)
    {
        write(1, "Latitude must be between [0; 90]", strlen("Latitude must be between [0; 90]"));
        return;
    }

    //introducem longitudine
    strcpy(msg,"Longitudine:");
    write(1,msg,strlen(msg));
    read(0,buffer,sizeof(buffer));
    new_treasure.longitude=atof(buffer);

    if(new_treasure.longitude>90.0)
    {
        write(1, "Longitude must be between [0; 180]", strlen("Longitude must be between [0; 180]"));
        return;
    }

    //introducem clue
    strcpy(msg,"Clue:");
    write(1,msg,strlen(msg));
    read(0,new_treasure.clue,sizeof(new_treasure.clue));
    new_treasure.clue[strcspn(new_treasure.clue, "\n")] = '\0';

    //introducem value
    strcpy(msg,"Value:");
    write(1,msg,strlen(msg));
    read(0,buffer,sizeof(buffer));
    new_treasure.value=atoi(buffer);

    if (write(treasure_file, &new_treasure, sizeof(new_treasure)) == -1) {
        perror("Error writing new treasure");
    }

    //pentru fisierul de log(acesta contine si symlink)
    log_operation(huntId,"added treasure");
    close(treasure_file);
    write(1, "The treasure has been added successfully.\n", strlen("The treasure has been added successfully.\n"));


}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        const char* msg = "trebuie trei argumente in linia de comanda\n";
        write(1, msg, strlen(msg));
        return 1;
    }

    if (strcmp(argv[1], "--add") == 0) 
    {
        add_treasure(argv[2]);
    }
    else if (strcmp(argv[1], "--list") == 0) 
    {
        list(argv[2]);
    }
    return 0;
}