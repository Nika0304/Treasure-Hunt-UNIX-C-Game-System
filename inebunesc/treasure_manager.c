#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include "treasure_manager.h"



//citirea unei linii
void readLine(char* buffer, size_t size) 
{
    ssize_t bytesRead = read(0, buffer, size - 1); 
    if (bytesRead > 0) 
    {
        buffer[bytesRead] = '\0';

        char* newline = strchr(buffer, '\n'); // Daca s-a citit cu eroare sau 0 bytes
        if (newline) {
            *newline = '\0';
        }
    } 
    else 
    {
        buffer[0] = '\0'; // Daca s-a citit cu eroare sau 0 
    }
}

// Functia care verifica daca un director exista
int DirectorulExista(const char *numeDirector) 
{
    struct stat st;
    return (lstat(numeDirector, &st) == 0 && S_ISDIR(st.st_mode));
}

// Functie care deschide un fisier si mesajul de erroare e in functie de fisierul pe care l-am deschis
int openFileWithCheck(const char* path, int flags, const char* errorMessage)
{
    int fd = open(path, flags, 0777);

    if (fd == -1) 
    {
        perror(errorMessage);
        exit(EXIT_FAILURE);
    }
    return fd;
}

//inchiderea unui file 
void closeFileWithCheck(int fd, const char* errorMessage) 
{
    if (close(fd) == -1) 
    {
        perror(errorMessage);
        exit(EXIT_FAILURE);          
    }
}

void getTreasureFilePath(const char* huntId, char* dest)
{
    snprintf(dest, MAX_PATH, "%s/treasures%s.dat", huntId, huntId);
}

void getLogFilePath(const char* huntId, char* dest)
{
    snprintf(dest, MAX_PATH, "%s/logged%s.txt", huntId, huntId);
}

void getLinkFilePath(const char* huntId, char* dest)
{
    snprintf(dest, MAX_PATH, "logged_hunt-%s", huntId);
}

//cream si verificam directorul cu numele huntId
void CreareVerificareDirector(const char* huntId)
{
    if (!DirectorulExista(huntId)) 
    {
        if ((mkdir(huntId, 0777)) == -1)
        {
            perror("Error creating directory");
            //exit(EXIT_FAILURE);
            return;
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
void createSymlink(const char* huntId,const char* log_path) 
{
    char link_path[MAX_PATH];
    getLinkFilePath(huntId,link_path);

    if (access(link_path,F_OK) == -1) //verifica daca link-ul exista deja
    {
        if (symlink(log_path,link_path) == -1) // daca nu exista incearca sa-l creeze
        {
            perror("error creating symlink");
            exit(EXIT_FAILURE);
        }
    }
}

//functia pentru fisierul de log specific fiecarui director
void log_operation(const char* hunt_id, const char* operation) 
{
    char log_path[MAX_PATH];
    getLogFilePath(hunt_id,log_path);

    int fd = openFileWithCheck(log_path, O_WRONLY | O_CREAT | O_APPEND, "Error opening the log");

    time_t now = time(NULL);  //obtine momentul curent de timp

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[%s] %s\n", strtok(ctime(&now), "\n"), operation);

    //creaza o legatura simbolica intre director si fisierul de log 
    createSymlink(hunt_id, log_path);

    write(fd, buffer, strlen(buffer));

    closeFileWithCheck(fd, "Error closing the log file");
}

void list(const char *huntId)
{
    char file_path[MAX_PATH];
    getTreasureFilePath(huntId, file_path);

    if(!DirectorulExista(huntId)) 
    {
        perror("This directory does not exist or is not a directory");
        //exit(EXIT_FAILURE);
        return;
    }

    //afisam numele directorului
    write(1, "HuntName: " ,strlen("HuntName: "));
    write(1, huntId, strlen(huntId));

    //verificare daca exista fisierul .dat
    struct stat st;
    if (lstat(file_path, &st) != 0 || !S_ISREG(st.st_mode)) 
    {
        perror("The file does not exist or is not a regular file");
        //exit(EXIT_FAILURE);
        return;
    }

    //afisare totalSize
    char totalSize[256];
    snprintf(totalSize, sizeof(totalSize), "\nTotal size in bytes: %lld\n", st.st_size);
    write(1, totalSize, strlen(totalSize));

    //afisare ultima modificare
    char lastModification[256];
    snprintf(lastModification, sizeof(lastModification), "Last modification : %s", ctime(&st.st_mtime));
    write(1, lastModification, strlen(lastModification));

    //deschidere fisier pentru citirea datelor despre treasures 
    int file = openFileWithCheck(file_path, O_RDONLY, "Error opening the treasure file in list");     

    //afisare tresures 
    Treasure t;

    lseek(file, 0, SEEK_SET);
    int find = 0;
    char buffer[256];
    write(1, "\nInformation about treasures\n", strlen("\nInformation about treasures\n"));
    ssize_t bytes_read;

    while((bytes_read = read(file, &t, sizeof(Treasure))) == sizeof(Treasure))
    {
        find++;
        int len = snprintf(buffer, sizeof(buffer), "Treasure: %d\n", find);
        write(1, buffer, len);
       
        len = snprintf(buffer, sizeof(buffer), "ID: %d\n", t.id);
        write(1, buffer, len);

        len = snprintf(buffer, sizeof(buffer), "UserName: %s\n", t.username);
        write(1, buffer, len);

        len = snprintf(buffer, sizeof(buffer), "Latitudine: %.2f -- Longitudine: %.2f\n",  t.latitude,t.longitude);
        write(1, buffer, len);      

        len = snprintf(buffer, sizeof(buffer), "Clue: %s\n",  t.clue);
        write(1, buffer, len); 

        len = snprintf(buffer, sizeof(buffer), "Value: %d\n", t.value);
        write(1, buffer, len);
       
        write(1, "___________________________________________\n", strlen("___________________________________________\n"));
    }

    if(find == 0)
    {
        write(1, "empty\n", strlen("empty\n"));
    }

    log_operation(huntId, "list hunt");
    closeFileWithCheck(file, "Error closing the treasure file");
}

//functie pentru vizualizarea datelor despre un anumit treasure
void view(const char *huntId, int id)
{
    char file_path[MAX_PATH];
    getTreasureFilePath(huntId, file_path);

    if(!DirectorulExista(huntId)) 
    {
        perror("This directory does not exist or is not a directory");
        //exit(EXIT_FAILURE);
        return;
    }

    //deschirderea fisierului de citire pt tresures
    int file = openFileWithCheck(file_path, O_RDONLY, "Error opening the treasure file in view");
    Treasure t;

    int found = 0;
    ssize_t bytes_read;

    while((bytes_read = read(file, &t, sizeof(Treasure))) == sizeof(Treasure))
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
        closeFileWithCheck(file, "Error closing the treasure file");
        return;
    }

    log_operation(huntId, "view treasure");
    closeFileWithCheck(file, "Error closing the treasure file");
}

//functie de verificare a id-ului ca sa fie unic
int isIdUnique(const char* huntId, int id)
{
    char file_path[MAX_PATH];
    getTreasureFilePath(huntId, file_path);

    int file = openFileWithCheck(file_path, O_RDONLY, "Error opening the treasure file in isIdUnique");

    Treasure t;
    ssize_t bytes_read;

    while ((bytes_read = read(file, &t, sizeof(Treasure))) == sizeof(Treasure))
    {
        if (t.id == id)
        {
            closeFileWithCheck(file, "Error closing the treasure file in isIdUnique");
            return 0; // ID-ul nu este unic
        }
    }

    closeFileWithCheck(file, "Error closing the treasure file in isIdUnique");
    return 1; // ID-ul este unic
}

//functie pentru citirea datelor unui trasure
////functie pentru citirea datelor unui trasure
//Daca datele introduse nu sunt valide (pentru tipurile int sau float), utilizatorul are posibilitatea sa le reintroduca pana cand sunt corecte(id, value, lat si long)
void add_treasure(const char* huntId)
{
    CreareVerificareDirector(huntId);

    char file_path[MAX_PATH];
    getTreasureFilePath(huntId, file_path);

    //deschidere fisier
    int treasure_file = openFileWithCheck(file_path, O_WRONLY  | O_CREAT | O_APPEND, "Error opening the treasure file in add_tresure ");

    Treasure new_treasure;
    char input[256];
    char *endptr = NULL;

    write(1, "Introduce treasure date below:\n", strlen("Introduce treasure date below:\n"));

    //introducem id
    do
    {
        write(1, "Id: ", strlen("Id: "));
        readLine(input, sizeof(input));

        new_treasure.id = strtol(input, &endptr, 10);

        // verific id e unic si un int >0 
        if (input[0] == '\0' || *endptr != '\0' ||  new_treasure.id < 0 )
        {
            write(1, "Id must be an int\n", strlen("Id must be an int\n"));
        }
        else if (!isIdUnique(huntId, new_treasure.id))
        {
            write(1, "ID already exists, please enter a unique ID.\n", strlen("ID already exists, please enter a unique ID.\n"));
            new_treasure.id = -1;
        }
    }
    while (input[0] == '\0' || *endptr != '\0' ||  new_treasure.id < 0);


    //introducem username
    write(1, "Username: ", strlen("Username: "));
    readLine(new_treasure.username, USERNAME_SIZE);

    //introducem latitudine ([0;90]lat N | S)
    do
    {
        write(1, "Latitude: ", strlen("Latitude: "));
        readLine(input, sizeof(input));

        new_treasure.latitude = strtof(input, &endptr);

        // verific daca latitudinea e int si intre 0 si 90
        if (*endptr != '\0' || new_treasure.latitude < 0.0 || new_treasure.latitude > 90.0 || input[0] == '\0')
        {
            write(1, "Latitude must be a number between [0; 90]\n", strlen("Latitude must be a number between [0; 90]\n"));
        }
    }
    while (new_treasure.latitude < 0.0 || new_treasure.latitude > 90.0 || input[0] == '\0' || *endptr != '\0');

    //introducem longitudine ([0;180]long E | V)
    do
    {
        write(1, "Longitude: ", strlen("Longitude: "));
        readLine(input, sizeof(input));

        new_treasure.longitude = strtof(input, &endptr);

        // verific daca longitudinea e int si intre 0 si 90
        if (*endptr != '\0' || new_treasure.longitude < 0.0 || new_treasure.longitude > 180.0 || input[0] == '\0')
        {
            write(1, "Longitude must be a number between [0; 180]\n", strlen("Longitude must be a number between [0; 180]\n"));
        }
    }
    while (new_treasure.longitude < 0.0 || new_treasure.longitude > 180.0 || input[0] == '\0' || *endptr != '\0');

    //introducem clue
    write(1, "Clue: ", strlen("Clue: "));
    readLine(new_treasure.clue, CLUE_SIZE);

    //introducem value
    do
    {
        write(1, "Value: ", strlen("Value: "));
        readLine(input, sizeof(input));

        new_treasure.value = strtol(input, &endptr, 10);

        // verific daca longitudinea e int si intre 0 si 90
        if (input[0] == '\0' || *endptr != '\0')
        {
            write(1, "Value must be an int\n", strlen("Value must be an int\n"));
        }
    }
    while (input[0] == '\0' || *endptr != '\0');

    
    if (write(treasure_file, &new_treasure, sizeof(new_treasure)) == -1) 
    {
        perror("Error writing new treasure");
    }

    //pentru fisierul de log(acesta contine si symlink)
    log_operation(huntId, "added treasure");

    closeFileWithCheck(treasure_file, "Error closing the treasure file");

    write(1, "The treasure has been added successfully.\n", strlen("The treasure has been added successfully.\n"));
}



//functie de stergere a unei comori
void remove_treasure(const char *huntId, int id_to_remove) 
{
    char file_path[MAX_PATH];
    getTreasureFilePath(huntId, file_path);

    if (!DirectorulExista(huntId)) 
    {
        write(1, "Directory doesn't exist.\n", strlen("Directory doesn't exist.\n"));
        return;
    }

    int fd = openFileWithCheck(file_path, O_RDWR, "Error opening the treasure file in remove_treasure"); //si pentru citire si pt scriere

    Treasure *treasures=NULL; 
    int count = 0;
    int buffer = 50;

    treasures = (Treasure*)malloc(sizeof(Treasure)*buffer);
    if(treasures == NULL)
    {
        perror("memory allocation error");
        closeFileWithCheck(fd, "Error closing the treasure file");
        exit(EXIT_FAILURE);
    }

    while (read(fd, &treasures[count], sizeof(Treasure)) == sizeof(Treasure)) 
    {
        count++;

        if(count == buffer)
        {
            buffer *= 2;
            treasures = (Treasure*)realloc(treasures,buffer*sizeof(Treasure));  
            if(treasures == NULL)
            {
                perror("memory reallocation error");
                free(treasures);
                closeFileWithCheck(fd, "Error closing the treasure file");
                exit(EXIT_FAILURE);
            }
        }
    }

    if(count != buffer)
    {
        treasures = (Treasure*)realloc(treasures,count*sizeof(Treasure));  
        if(treasures == NULL)
        {
            perror("memory reallocation error");
            free(treasures);
            closeFileWithCheck(fd, "Error closing the treasure file");
            exit(EXIT_FAILURE);
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
        free(treasures);
        closeFileWithCheck(fd, "Error closing the treasure file");
        return;
    }

    lseek(fd, 0, SEEK_SET); 
    for (int i = 0; i < count; i++) 
    {
        if (write(fd, &treasures[i], sizeof(Treasure)) == -1) 
        {
            perror("Error writing to file");
            free(treasures);
            closeFileWithCheck(fd, "Error closing the treasure file");
            exit(EXIT_FAILURE);
        }
    }

    if (ftruncate(fd, count * sizeof(Treasure)) == -1) 
    {
        perror("Error truncating file");
        free(treasures);
        closeFileWithCheck(fd, "Error closing the treasure file");
        exit(EXIT_FAILURE);
    } 
    else 
    {
        write(1, "Treasure successfully removed.\n", strlen("Treasure successfully removed.\n"));
        log_operation(huntId, "removed treasure");
    }

    free(treasures);
    closeFileWithCheck(fd, "Error closing the treasure file");
}

//functie de stergere a unui hunt
void remove_hunt(const char *huntId)
{
    char file_path[MAX_PATH];
    getTreasureFilePath(huntId,file_path);

    char log_path[MAX_PATH];
    getLogFilePath(huntId,log_path);

    char link_path[MAX_PATH];
    getLinkFilePath(huntId,link_path);

    if (!DirectorulExista(huntId)) 
    {
        write(1, "Directory doesn't exist.\n", strlen("Directory doesn't exist.\n"));
        return;
    }

    struct stat st;

    //stergere legatura simbolica
    if (lstat(link_path, &st) == 0 && S_ISLNK(st.st_mode) != 0)
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

    //sterge fisierul de treasures
    if (lstat(file_path, &st) == 0 && S_ISREG(st.st_mode) != 0)
    {
        if (remove(file_path) == -1) {
            perror("Error removing treasures file");
            return;
        } 
        else 
        {
            write(1, "Treasure file removed successfully.\n", strlen("Treasure file removed successfully.\n"));
        }
    }

    //sterge fisierul de log
    if (lstat(log_path,&st) == 0 && S_ISREG(st.st_mode) != 0)
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

    //sterge directorul
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

