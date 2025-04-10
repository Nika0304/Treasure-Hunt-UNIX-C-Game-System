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
//de aduagat ID unic + daca nu intriduce o data corecta sa mai poata introduce fara sa se termine programul + sa adaug țn la sfarsit de linie
//sa incerc sa organizez pe functii fara sa duplic cod (de exemplu la deschiderea unui fisier, cale pentru director etc.)


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

int openTreasureFile(const char *path, int flags)
{
    int fd = open(path, flags, 0777);
    if (fd == -1) 
    {
        perror("Error opening treasure file");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void getTreasureFilePath(const char* huntId, char* dest)
{
    snprintf(dest, MAX_PATH, "%s/treasures%s.dat", huntId, huntId);
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
void createSymlink(const char* huntId,const char *log_path) 
{
    char link_path[MAX_PATH];
    snprintf(link_path, sizeof(link_path), "logged_hunt-%s", huntId);
    if(access(link_path,F_OK) == -1)
    {
        if(symlink(log_path,link_path) == -1)
        {
            perror("error creating symlink");
            exit(-1);
        }
    }
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

    time_t now = time(NULL);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[%s] %s\n", strtok(ctime(&now), "\n"), operation);

    write(fd, buffer, strlen(buffer));
    close(fd);

}

void list(const char *huntId)
{
    char file_path[MAX_PATH];
    getTreasureFilePath(huntId, file_path);

    if(!DirectorulExista(huntId)) 
    {
        perror("This directory does not exist or is not a directory");
        exit(-1);
    }

    //afisam numele directorului
    write(1, "HuntName: " ,strlen("HuntName: "));
    write(1, huntId, strlen(huntId));

    //verificare daca exista fisierul .dat
    struct stat st;
    if (lstat(file_path, &st) != 0 || !S_ISREG(st.st_mode)) 
    {
        perror("The file does not exist or is not a regular file");
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
    int file=openTreasureFile(file_path,O_RDONLY);

    //int file = open(file_path, O_RDONLY);
     

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

    if(find == 0)
    {
        write(1, "empty\n", strlen("empty\n"));
    }
    free(t);
    log_operation(huntId, "list hunt");
    close(file);
}

//functie pentru vizualizarea datelor despre un anumit hunt

void view(const char *huntId, int id)
{
    char file_path[MAX_PATH];
    getTreasureFilePath(huntId, file_path);

    if(!DirectorulExista(huntId)) 
    {
        perror("This directory does not exist or is not a directory");
        exit(-1);
    }

    int file = openTreasureFile(file_path,O_RDONLY);
    Treasure t;
    int found=0;

    while(read(file,&t,sizeof(Treasure))==sizeof(Treasure))
    {
        if(t.id == id)
        {
            found=1;
            char str[512];
            snprintf(str, sizeof(str), "ID: %d\nUser_Name: %s\nLatitude: %.2f\nLongitude: %.2f\nClue: %s\nValue: %d\n", t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
            write(1, str, strlen(str));
            break;
        }
    }
    if (found == 0)
    {
        write(1, "Nu a fost gasit id-ul\n", strlen("Nu a fost gasit id-ul\n"));
        return;
    }

    close(file);
    log_operation(huntId,"view treasure");
}
//functie pentru citirea datelor unui trasure
void add_treasure(const char *huntId)
{
    CreareVerificareDirector(huntId);

    char file_path[MAX_PATH];
    getTreasureFilePath(huntId, file_path);

    int treasure_file=openTreasureFile(file_path, O_WRONLY  | O_CREAT | O_APPEND);

    Treasure new_treasure;
    char buffer[32];
    write(1, "Introduce treasure date below:\n", strlen("Introduce treasure date below:\n"));

    char msg[50];
    //introducem id
    strcpy(msg, "ID:");
    write(1, msg, strlen(msg));
    read(0, buffer, sizeof(buffer));
    new_treasure.id=atoi(buffer);

    //introducem username
    strcpy(msg, "Username:");
    write(1, msg, strlen(msg));
    read(0, new_treasure.username, sizeof(new_treasure.username));
    new_treasure.username[strcspn(new_treasure.username, "\n")] = '\0';

    //introducem latitudine
    strcpy(msg, "Latitudine:");
    write(1, msg, strlen(msg));
    read(0, buffer, sizeof(buffer));
    new_treasure.latitude = atof(buffer);

    if(new_treasure.latitude > 90.0)
    {
        write(1, "Latitude must be between [0; 90]", strlen("Latitude must be between [0; 90]"));
        return;
    }

    //introducem longitudine
    strcpy(msg, "Longitudine:");
    write(1, msg, strlen(msg));
    read(0, buffer, sizeof(buffer));
    new_treasure.longitude = atof(buffer);

    if(new_treasure.longitude > 180.0)
    {
        write(1, "Longitude must be between [0; 180]", strlen("Longitude must be between [0; 180]"));
        return;
    }

    //introducem clue
    strcpy(msg, "Clue:");
    write(1, msg, strlen(msg));
    read(0, new_treasure.clue, sizeof(new_treasure.clue));
    new_treasure.clue[strcspn(new_treasure.clue, "\n")] = '\0';

    //introducem value
    strcpy(msg, "Value:");
    write(1, msg, strlen(msg));
    read(0, buffer, sizeof(buffer));
    new_treasure.value = atoi(buffer);

    if (write(treasure_file, &new_treasure, sizeof(new_treasure)) == -1) {
        perror("Error writing new treasure");
    }

    //pentru fisierul de log(acesta contine si symlink)
    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "%s/logged%s.txt", huntId,huntId);
    log_operation(huntId, "added treasure");
    createSymlink(huntId,log_path);
    close(treasure_file);
    write(1, "The treasure has been added successfully.\n", strlen("The treasure has been added successfully.\n"));


}



//functie de stergere a unei comori
void remove_treasure(const char *huntId, int id_to_remove) {
    char file_path[MAX_PATH];
    getTreasureFilePath(huntId, file_path);

    if (!DirectorulExista(huntId)) {
        write(1, "Directory doesn't exist.\n", strlen("Directory doesn't exist.\n"));
        return;
    }

    int fd = openTreasureFile(file_path, O_RDWR); 

    Treasure *treasures=NULL; 
    int count = 0;
    int buffer = 50;

    treasures = (Treasure*)malloc(sizeof(Treasure)*buffer);
    if(treasures == NULL)
    {
        perror("eroare la alocarea memoriei");
        close(fd);
        exit(-1);
    }

    while (read(fd, &treasures[count], sizeof(Treasure)) == sizeof(Treasure)) {
        count++;

        if(count == buffer)
        {
            buffer *= 2;
            treasures = (Treasure*)realloc(treasures,buffer*sizeof(Treasure));  
            if(treasures == NULL)
            {
                perror("eroare la realocarea memoriei");
                close(fd);
                exit(-1);
            }
        }
    }

    if(count != buffer)
    {
        treasures = (Treasure*)realloc(treasures,count*sizeof(Treasure));  
        if(treasures == NULL)
        {
            perror("eroare la realocarea memoriei");
            close(fd);
            exit(-1);
        }
    }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (treasures[i].id == id_to_remove) 
        {
            found = 1;
            for (int j = i; j < count - 1; j++) 
            {
                treasures[j] = treasures[j + 1];
            }
            count--; 
            break;
        }
    }

    if (!found) {
        write(1, "Treasure with specified ID not found.\n", strlen("Treasure with specified ID not found.\n"));
        close(fd);
        return;
    }

    lseek(fd, 0, SEEK_SET); 
    for (int i = 0; i < count; i++) 
    {
        if (write(fd, &treasures[i], sizeof(Treasure)) == -1) 
        {
            perror("Error writing to file");
            close(fd);
            exit(-1);
        }
    }

    if (ftruncate(fd, count * sizeof(Treasure)) == -1) 
    {
        perror("Error truncating file");
    } else 
    {
        write(1, "Treasure successfully removed.\n", strlen("Treasure successfully removed.\n"));
        log_operation(huntId, "removed treasure");
    }

    free(treasures);
    close(fd);
}

//functie de stergere a unui hunt
void remove_hunt(const char *huntId)
{
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/treasures%s.dat", huntId, huntId);

    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "%s/logged%s.txt", huntId, huntId);

    char link_path[MAX_PATH];
    snprintf(link_path, sizeof(link_path), "logged_hunt-%s", huntId);

    if (!DirectorulExista(huntId)) 
    {
        write(1, "Directory doesn't exist.\n", strlen("Directory doesn't exist.\n"));
        return;
    }

    struct stat st;

    //stergere legatura simbolica
    if(lstat(link_path,&st) == 0 && S_ISLNK(st.st_mode) != 0)
    {
        if (unlink(link_path) == -1) 
        {
            perror("Error removing symlink");
            return;
        } 
        else
        {
            write(1, "Symlink removed successfully.\n", strlen("Symlink removed successfully.\n"));
        }
    }

    if(lstat(file_path,&st) == 0 && S_ISREG(st.st_mode) != 0)
    {
        if (remove(file_path) == -1) {
            perror("Error removing treasure file");
            return;
        } 
        else 
        {
            write(1, "Treasure file removed successfully.\n", strlen("Treasure file removed successfully.\n"));
        }
    }

    if(lstat(log_path,&st) == 0 && S_ISREG(st.st_mode) != 0)
    {
        if (remove(log_path) == -1) {
            perror("Error removing log file");
            return;
        } 
        else 
        {
            write(1, "Log file removed successfully.\n", strlen("Log file removed successfully.\n"));
        }
    }

    if (rmdir(huntId) == -1) 
    {
        perror("Error removing hunt directory");
        return;
    } 
    else 
    {
        write(1, "Hunt directory removed successfully.\n", strlen("Hunt directory removed successfully.\n"));
    }

}


int main(int argc, char* argv[]) {
    if(argc==3)
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

    }
    else if (argc == 4)
    {
        if (strcmp(argv[1], "--view") == 0) 
        {
            view(argv[2],atoi(argv[3]));
        }
        if (strcmp(argv[1], "--remove_treasure") == 0) 
        {
            remove_treasure(argv[2],atoi(argv[3]));
        }
    }
    return 0;
}