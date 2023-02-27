#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX 5

typedef struct hashLink {
    char name[64];
    int number;
    bool exists;
    struct hashLink *next;
} hashLink;
typedef struct hashMap {
    struct hashLink table[MAX];
} hashMap;

void insert(int, char*, hashMap*);
int hash(char*);
void initialize(hashMap*);

int main(int argc, char *argv[] ){
    char* fileName = malloc(strlen(argv[1]) + 1); 
    strcpy(fileName, argv[1]);
    FILE *fp;
    char buffer[255];
    char *token;
    int num = 0;
    bool hasNum = false;
    hashMap mappy;
    
    initialize(&mappy);
    fp = fopen ( fileName, "r");
    while (fgets (buffer, 255, fp) != NULL) {
        printf("buffer: %s\n", buffer);
        token = strtok(buffer, " ");
        char *name = malloc(strlen(token) + 1); // a char is one byte lol
        strcpy(name, token);
        if (name[strlen(name)-1] == '\n') {
            name[strlen(name) - 1] = '\0';
        }
        if ((token = strtok(NULL, " ")) != NULL) {
            hasNum = true;
            num = atoi(token);
        } else {
            hasNum = false;
            num = 0;
        }
        // printf("----------\n"
        //        "name   : %s\n"
        //        "number : %d\n"
        //        "hashval: %d\n"
        //        "----------\n", name, num, hash(name));
        insert(num, name, &mappy);
        free(name);
    }
    //printf("%s\n", buffer);
   
    return 0;
}
/*
    Takes:
        - int num, char* name: future data members of a hashMap element
        - hashMap mappy: the hashMap in question
    Calls: 
        - hash
    Returns:
        none
    Does:
        check if there is number
        if input has number
            if already exists
                report error
            if not
                insert new link
                print that new link was inserted
        if input has no number
            if input == value at location
                print out that you found the input at that location
            if input not found
                print not found
*/
void insert(int num, char* name, hashMap *mappy) {
    hashLink link;
    hashLink oldLink;
    int hashVal = hash(name);
    // how to check if collision?
    oldLink = mappy->table[hashVal]; // this ain't it chief 
    printf("----------------------------------\n");
    if (num) {
        if (oldLink.exists) {
            printf("Collion: %s found at location %d with number %d\n", oldLink.name, hashVal, oldLink.number);
            printf("[%s %d] not inserted because the hashVal is %d\n", name, num, hashVal);
        } else {
            strcpy(link.name,name);
            link.number = num;
            link.exists = true;
            mappy->table[hashVal] = link;
            printf("Stored %s %d at index %d\n", name, num, hashVal);
        }
    } else { // if the input has no number and is just name
        if (strcmp(name,oldLink.name)!=0) { // evaluates true if the strings are not the same
            printf("%s not found, oldLink.name is %s\n",name,oldLink.name);
        } else {
            printf("%s found at index %d with value %d \n", name, hashVal, oldLink.number);
        }
    }
    printf("----------------------------------\n");
}
/*
    sbdm algorithm from https://stackoverflow.com/questions/14409466/simple-hash-functions
*/
int hash(char* name){
    int value = 0; // gotta initialize it
    for (int i = 0; i < strlen(name); i++) {
        value = name[i] + (value << 6) + (value << 16) - value;
    }
    return value % (MAX);
}
/*
    because structs are not classes
*/
void initialize(hashMap *mappy) {
    for(int i = 0; i < MAX; i++) {
        strcpy(mappy->table[i].name, " ");
        mappy->table[i].number = -1;
        mappy->table[i].exists = false;
    }
}