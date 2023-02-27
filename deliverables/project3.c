#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX 255

struct hashLink {
    char *label;
    int address;    // base 10
    bool extended;
    bool indirect;
    bool immediate;
    struct hashLink *next;
};
 struct opcode {
    char *mneum;
    char *opcode;
    int format;
    struct opcode *next;
};

struct opcode *opTab[MAX];
struct hashLink *hashTable[MAX];


void lineProcess(char*, char*, char*, char*, char*, bool*, bool*, bool*);
struct hashLink* lookup(char*);
struct opcode* opLookup(char *in);
struct hashLink *insert(char *label, int address);
int hash(char*);
char* decHex(int);
void optabInit();
void initialize();
char * trim(char*);
void dumpSymbols();
int biteProcessing(char *in);

int main (int argc, char *argv[] ) {
    char* fileName = malloc(strlen(argv[1]) + 1); 
    strcpy(fileName, argv[1]);
    //char fileName[13] = "Testfile.txt";
    char start[6] = "START";
    char base[5] = "BASE";
    char resw[5] = "RESW";
    char resb[5] = "RESB";
    char word[5] = "WORD";
    char byte[5] = "BYTE";
    char end[4] = "END";
    FILE *fp;
    FILE *rf;
    char buffer[80];
    char bufClone[80];
    unsigned locctr = 0;
    unsigned prevLoc = 0;
    struct opcode *op;
    bool isLabel;

    
    
    char label[9]; // cols 0-7
    bool extended = false;
    bool indirect = false;  // for @
    bool immediate = false; // for #
    char mneum[8]; // cols 10-16
    char operand[11];
    char comments[49];
    char *token;

    
    initialize();
    optabInit();
    //opLookup("LDB");
    fp = fopen(fileName, "r");

    // TODO: Make intermediate file
    rf = fopen("results.txt", "w");
    // fprintf(rf, "%s", start);
    // fclose(rf);
    // TODO: Store SICOPS.txt in a usable format
    


    /*
        buffer  [0-7] : optional label
        buffer    [8] : blank
        buffer    [9] : optional +
        buffer[10-16] : mneumonic
        buffer   [17] : blank
        buffer   [18] : optional @,#,=
        buffer[19-28] : operand (label, register, ',')
        buffer[29-30] : blank
        buffer[31-79] : comments
    */
    while (fgets (buffer, 255, fp) != NULL) {   // this should be in a function
        strcpy(bufClone,buffer);
        if (buffer[0] == '.' || isspace(buffer[10])) {
            printf("%s\n", buffer);
            fprintf(rf, "%s", buffer);
        } else {
            lineProcess(bufClone, label, mneum, operand, comments, &extended, &indirect, &immediate);
            trim(mneum);
            // printf("%X\n", locctr);
            // printf("%s\n", buffer);
            // printf("label : %s<======end\n", label);
            // printf("mneuumonic : |%s|\n", mneum); 
            // if(indirect) printf("indirect(@) \n");
            // if(immediate) printf("immediate(#) \n");
            // printf("operand: %s\n", operand);
            // printf("comments: %s\n", comments);
            // printf("%s %d\n", mneum, op->format);
            if (isspace(label[0])) {
                isLabel = false;
            } else isLabel = true;
            
            if (strcmp(mneum, start) == 0)  {   // for START only
                locctr = (int)strtol(operand, NULL, 16);
                if (isLabel) {
                    insert(label, locctr);
                }
                printf("%4X   %s\n", locctr, buffer);
                fprintf(rf, "%4X   %s", locctr, buffer);
            } else if (strcmp(mneum, base) == 0) {  // for BASE only
                printf("%4X   %s\n", locctr, buffer);
                fprintf(rf, "%4X   %s", locctr, buffer);
                if (isLabel) {
                    insert(label, locctr);
                }
            } else if (strcmp(mneum,resw) == 0) {   // RESW
                if (isLabel) {
                    insert(label, locctr);
                }
                printf("%4X   %s\n", locctr, buffer);
                fprintf(rf, "%4X   %s", locctr, buffer);
                locctr += 3 * atoi(operand);
            } else if (strcmp(mneum,resb) == 0) {   // RESB
                if (isLabel) insert (label, locctr);
                printf("%x   %s\n", locctr, buffer);
                fprintf(rf, "%x   %s", locctr, buffer);
                locctr += atoi(operand);
            } else if (strcmp(mneum, word) == 0) {  // WORD
                if (isLabel) insert (label, locctr);
                printf("%4X   %s\n", locctr, buffer);
                fprintf(rf, "%4X   %s", locctr, buffer);
                locctr += 3;
            } else if (strcmp(mneum, byte) == 0) {  // BYTE
                if (isLabel) insert (label, locctr);
                printf("%4X   %s\n", locctr, buffer);
                fprintf(rf, "%4X   %s", locctr, buffer);
                locctr += biteProcessing(operand); 
            } else if (strcmp(mneum,end) == 0) {    // END
                printf("%4X   %s\n", locctr, buffer);
                fprintf(rf, "%4X   %s", locctr, buffer);
            } else {    // for instructions not reserving stuff
                op = opLookup(mneum);
                printf("%4X   %s\n", locctr, buffer);
                fprintf(rf, "%4X   %s", locctr, buffer);
                if (isLabel) {
                    insert(label, locctr);
                }
                locctr += op->format;
                if(extended) locctr++;
                
            }
            
            prevLoc = locctr;
        }
        immediate  = false;
        indirect = false;
        extended = false;
    }
    fclose(rf);
    fclose(fp);
    dumpSymbols();
    return 0;
}

// process each line to load variables with useful data
void lineProcess(char* buffer, char* label, char* mneum, char* operand, char* comments, bool* extended, bool* indirect, bool* immediate) {
    // printf("---------------------\n");
    // printf("buffer: %s\n", buffer);

    for (int i = 0; i < 8; i++) {           // label
        label[i] = buffer[i];
        //printf("label[%d] = %c \n", i, label[i]);
    }
    label[9] = '\0';

    if (buffer[9] == '+') *extended = true;  // extended
    
    for (int i = 10; i<17;i++) {            // mneumonic
        mneum[i-10] = buffer[i];
    }
    mneum[7] = '\0';
    trim(mneum);


    // printf("buffer[18] = %c\n", buffer[18]);// indirect/immdiate
    if (buffer[18] == '@') {
        *indirect = true;
    } else if(buffer[18] == '#') {
        *immediate = true;
    }

    for (int i = 19; i < 29; i++) {         // operand
        operand[i-19] = buffer[i];
    }
    operand[10] = '\0';

    for (int i = 31; i < 80; i++) {         // comments
        comments[i-31] = buffer[i];
    }
    comments[48] = '\0';
    trim(comments);
    

    // printf("%s\n", buffer);
    // printf("label : %s\n", label);
    // printf("mneuumonic : %s\n", mneum);
    // if(indirect) printf("indirect(@) \n");
    // if(immediate) printf("immediate(#) \n");
    // printf("operand: %s\n", operand);
    // printf("comments: %s\n", comments);
}

/* 
     returns pointer to location of label in table if found, NULL if not.
     taken from "The C Programming Language, 2nd edition" 
     by Brian Kernighan and Dennis Ritchie
*/
struct hashLink* lookup(char *in) {
    struct hashLink *np;

    for (np = hashTable[hash(in)]; np != NULL; np = np->next) {
        if (strcmp(in, np->label) == 0)
            return np;
    }
    return NULL;
}
void dumpSymbols() {
    FILE *rf;
    rf = fopen("results.txt", "a");
    struct hashLink *np;
    fprintf(rf, "\nINDEX LABEL    ADDRESS\n");
    printf("INDEX   LABEL    ADDRESS\n");
    for (int i = 0; i <= MAX; i++) { 
        //printf("%d\n", i);
        for (np = hashTable[i]; np != NULL; np = np->next) {
            fprintf(rf, "%3d   %s  %x\n", i, np->label, np->address);
            printf("%-d   %s  %x\n", i, np->label, np->address);
        }
    }
}
/*
    Returns opcode from table based on mnuemonic 
*/
struct opcode* opLookup(char *in) {
    struct opcode *np;

    //printf("hashVal: %d\n", hash(in));
    
    for (np = opTab[hash(in)]; np != NULL; np = np->next) {
        //printf("lookedup, np->mneum: %s, %s\n", in, np->mneum);
        if (strcmp(in, np->mneum) == 0)  {
            //printf("%s, %s, %d\n", np->mneum, np->opcode, np->format);
            return np;
        }
    }
    return NULL;
}
/*
    Taken directly from "The C Programming Language, 2nd edition"
*/
struct hashLink *insert(char *in, int address) {
    struct hashLink *np;
    char *lab;
    int hashval;
    //char *lab = trim(label);
    lab = malloc(sizeof(in));
    strcpy(lab, in);
    lab[strlen(lab)] = '\0';
    //printf("lablad%s\n", lab);
    
    if ((np = lookup(lab)) == NULL) { /* not found */
        np = (struct hashLink *) malloc(sizeof(*np));
        //printf("lablad%s\n", lab);
        np->label = lab;
        np->address = address;
        hashval = hash(lab);
        np->next = hashTable[hashval];
        hashTable[hashval] = np;
    } else /* already there */
        printf("label already found in hashTable[%d]\n", hashval); 
    // if ((np->defn = strdup(defn)) == NULL)
    //     return NULL;
    free(lab);
    return np;
}

/*
    sbdm algorithm from https://stackoverflow.com/questions/14409466/simple-hash-functions
*/
int hash(char* name){
    unsigned value = 5381;
        for (int i = 0; name[i]!='\0'; i++){
            value = ((value << 5) + value) + name[i];
        }
    //printf("value: %d\n", value);
    return value % (MAX);
}
/*
    from https://www.sanfoundry.com/c-program-convert-decimal-hex/
    I did not need to implement this
*/
char* decHex(int decimal){
    char hex[8];
    int quotient, remainder, i = 0;

    quotient = decimal;

    while (quotient != 0) {
        remainder = quotient % 16;
        if (remainder < 10 ) {
            hex[i++] = 48 + remainder;
        } else {
            hex[i++] = 55 + remainder;
        }
        quotient = quotient / 16;
    }
    return strdup(hex);
}
/*
    because structs are not classes
*/
void optabInit() {
    char fileName[11] = "SICOPS.txt";
    FILE *fp;
    fp = fopen(fileName, "r");
    char buffer[22];
    char *token;
    char *noomonic;
    char *code;
    int hashVal = 0;
    struct opcode *op;
   

    /*
        get mnuemonic, 2digit opcode, and format
    */
    while (fgets (buffer, 255, fp) != NULL) {
        hashVal = 0;
        noomonic = NULL;
        code = NULL;
        token = strtok(buffer, " ");
        //printf("token: %s\n", token);
        noomonic = malloc(sizeof(token)); 
        strcpy(noomonic, token);
        
        noomonic[strlen(noomonic)] = '\0';
        //printf("%s\n", noomonic);
        op = (struct opcode *) malloc(sizeof(*op));
        op->mneum = noomonic;

        token = strtok(NULL, " ");
        // printf("token: %s\n", token);
        code = malloc(strlen(token) + 1); 
        strcpy(code, token);
        code[strlen(code)] = '\0';
        op->opcode = strdup(code);

        if (strlen(noomonic) < 3) {  // is a register
            op->format = 0;
        } else {                    // is an instruction
            token = strtok(NULL, " ");
            // printf("token: %s\n", token);
            op->format = atoi(token);
        }
        hashVal = hash(noomonic);
        //printf("hashval: %d\nformat: %d\nopcode: %s\nmneum:%s\n", hashVal, op->format, op->opcode, op->mneum);
        op->next = opTab[hashVal];
        opTab[hashVal] = op;
        token = NULL;

        // if ((token = strtok(NULL, " ")) != NULL) {
        //     num = atoi(token);
        // }
    }
    free(noomonic);
    fclose(fp);

}
void initialize() {
    struct hashLink *hl; 
    for(int i = 0; i < MAX; i++) {
        hashTable[i] = NULL;
        opTab[i] = NULL;
    }
}
char * trim(char *c) {
    char * e = c + strlen(c) - 1;
    while(*c && isspace(*c)) c++;
    while(e > c && isspace(*e)) *e-- = '\0';
    return c;
}
int biteProcessing(char *in) {
    char* st;
    st = malloc(sizeof(in));
    int len = strlen(st);
    strcpy(st, in);
    st[len] = '\0';
    trim(st);

    if(st[0] == 'C') {
        free(st);
        return (len - 3);
    }   else if (st[0] == 'X') {
        free(st);
        //printf("strlen is by: %d\n", strlen(st));
        return (len - 3) / 2;
    }
}