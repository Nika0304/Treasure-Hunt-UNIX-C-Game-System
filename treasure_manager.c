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
// sa inteb daca pot folosi printf pentru mesaje simple !!!!! daca nu trebuie sa schimb

void CreareVerificareDirector(const char *huntId)
{
    //cream si verificam directorul cu numele huntId
    struct stat st;
    if(lstat(huntId,&st)==-1 || !S_ISDIR(st.st_mode)) //verific daca directorul nu exista sau nu este director
    {
        if ((mkdir(huntId,0777))==-1)
        {
            perror("error creating directory");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Directorul a fost creat cu succes\n");
        }
    }
    else
    {
        printf("directorul deja exista\n");
    }
}

//functie pentru citirea datelor unui trasure
Treasure add_treasure(const char *huntId)
{
    Treasure new_treasure;
    char buffer[32];
    printf("Introduce trasure date below:\n");

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

    //introducem longitudine
    strcpy(msg,"Longitudine:");
    write(1,msg,strlen(msg));
    read(0,buffer,sizeof(buffer));
    new_treasure.longitude=atof(buffer);

    //introducem clue
    strcpy(msg,"Clue:");
    write(1,msg,strlen(msg));
    read(0,new_treasure.clue,sizeof(new_treasure.clue));
    new_treasure.clue[strcspn(new_treasure.clue, "\n")] = '\0';

    //introducem longitudine
    strcpy(msg,"Value:");
    write(1,msg,strlen(msg));
    read(0,buffer,sizeof(buffer));
    new_treasure.value=atoi(buffer);

    return new_treasure;
}

//functia pentru fisierul de log specific fiecarui director
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
}

void createSymlink(const char* huntId) {
    //inca nu stiu cum se face :((; intreaba miercuri la lab

    //trebuie sa parcurg tot directorul si sa creez logged_hunt-<ID>.(acesta e numele simbolic si creaza legatura cu fisierul meu de log)
    //daca am inteles bine logged_hunt-<ID> din directorul rădăcină al programului va face referire la fișierul original logged_hunt din fiecare director hunt.
    //exemplu logged_hunt-hunt1 va fi o legătură simbolică care va face referire la hunt1/loggedhunt1.txt.

    //nu vreauuuuuuuuuuuuuuuuuuu
}

void list(int file)
{
    Treasure *t=(Treasure*)malloc(sizeof(Treasure));
    if(t==NULL)
    {
        perror("Eroare la alocarea memoriei");
        exit(EXIT_FAILURE);
    }
    lseek(file,0,SEEK_SET);
    int find=0;
    printf("\nInformatiile despre comurile\n");
    while(read(file,t,sizeof(*t))==sizeof(*t))
    {
        find++;
        printf("\n");
        printf("Treasure %d\n",find);
        printf("ID: %d\n", t->id);
        printf("Username: %s\n", t->username);
        printf("Latitudine: %.2f  -- Longitudine: %.2f\n", t->latitude,t->longitude);
        printf("Clue: %s\n", t->clue);
        printf("Value: %d\n", t->value);
        printf("___________________________________________\n");
    }

    if(find==0)
    {
        printf("fisier gol\n");
    }
    free(t);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        const char* msg = "trebuie trei argumente in linia de comanda\n";
        write(1, msg, strlen(msg));
        return 1;
    }

    if (strcmp(argv[1], "--add") == 0) 
    {
        //pentru fisierul de tresures 
        CreareVerificareDirector(argv[2]);
        char file_path[MAX_PATH];
        snprintf(file_path,MAX_PATH,"%s/treasures%s.txt",argv[2],argv[2]);
        int treasure_file=open(file_path, O_RDWR | O_CREAT | O_APPEND, 0777);
        if (treasure_file==-1)
        {
            perror("Error opening the treasure file");
            close(treasure_file);
            return 1;
        }
        Treasure new_treasure=add_treasure(argv[2]);
        if (write(treasure_file, &new_treasure, sizeof(new_treasure)) == -1) {
            perror("Error writing new treasure");
        }

        //pentru fisierul de log
        log_operation(argv[2],"added treasure");
        list(treasure_file);
        //mai e nevoie de createSymLink
    }
    return 0;
}