#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "treasure_manager.h"

typedef struct {
    char username[USERNAME_SIZE];
    int score;
} UserScore;

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        printf("introdu si numele hunt", argv[0]);
        return 1;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/treasures%s.dat", argv[1], argv[1]);

    int f = openFileWithCheck(path, O_RDONLY, "error");

    Treasure t;
    UserScore scores[100];
    int count = 0;

    ssize_t bytes_read;
    while ((bytes_read = read(f, &t, sizeof(Treasure))) == sizeof(Treasure)) 
    {
        int found = 0;

        // Find if the user already exists in the scores list
        for (int i = 0; i < count; ++i) 
        {
            if (strcmp(scores[i].username, t.username) == 0) 
            {
                // If user found, add the treasure value to their score
                scores[i].score += t.value;
                found = 1;
                break;
            }
        }

        // If user is not found, add them to the list
        if (!found && count < 100) {
            strcpy(scores[count].username, t.username);
            scores[count].score = t.value;
            count++;
        }
    }

    closeFileWithCheck(f, "error");

    // Display the total score for each user
    if (count == 0) 
    {
        printf("No treasures found or no scores calculated.\n");
    } 
    else 
    {
        for (int i = 0; i < count; ++i) 
        {
            printf("Hunt %s: User %s => Score: %d\n", argv[1], scores[i].username, scores[i].score);
        }
    }

    return 0;
}
