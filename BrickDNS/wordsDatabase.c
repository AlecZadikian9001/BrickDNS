//
//  wordsDatabase.c
//  BrickDNS
//
//  Created by Alec Zadikian on 10/4/14.
//  Copyright (c) 2014 AlecZ. All rights reserved.
//

#include "wordsDatabase.h"

int loadWordList(sqlite3* db, int* longestWord){
    int longest = 0;
    char createWordTable[] = "CREATE TABLE 'WORDS' ('word' char(1024) PRIMARY KEY UNIQUE NOT NULL, 'type' int(3) NOT NULL, 'typeIndex' int(6) NOT NULL)";
    databaseCommand(createWordTable, db);
    char addToTableFormat[] = "INSERT INTO 'WORDS' ('word', 'type', 'typeIndex') VALUES ('%s' , '%d', '%d')";
    char commandBuffer[1024];
    FILE *ifp;
    char *mode = "r";
    ifp = fopen("dictionary.txt", mode); //
    if (ifp == NULL) {
        fprintf(stderr, "Can't open input file in.list!\n");
        return RET_INPUT_ERROR;
    }
    char wordBuffer[128];
    int bigIndex = 0;
    int length;
    int type = 0;
    //N, V, t, i, A, v
    int types[6] = {0,0,0,0,0,0};
    while (fscanf(ifp, "%s\r", wordBuffer) != EOF) {
        length = strlen(wordBuffer);
        for (int i = 0; i<strlen(wordBuffer); i++){
            if (wordBuffer[i]==' ' || (wordBuffer[i]>='A' && wordBuffer[i]<='Z')) break;
            if (wordBuffer[i]=='\xd7'){
                wordBuffer[i] = '\0';
                i++;
                type = wordBuffer[i];
                int typeIndex = 0;
                bool valid = true;
                if (type=='N') typeIndex = 0;
                else if (type=='V') typeIndex = 1;
                else if (type=='t') typeIndex = 2;
                else if (type=='i') typeIndex = 3;
                else if (type=='A') typeIndex = 4;
                else if (type=='v') typeIndex = 5;
                else{
                    valid = false;
                }
                if (valid){
                    types[typeIndex] = types[typeIndex]+1;
                    if (strlen(wordBuffer) > longest) longest = strlen(wordBuffer);
                }
                sprintf(commandBuffer, addToTableFormat, wordBuffer, type, types[typeIndex]);
                databaseCommand(commandBuffer, db);
                break;
            }
        }
        bigIndex++;
        if (bigIndex%1000==0){
            printf("Scanned %d words thus far...\n", bigIndex);
        }
    }
    printf("Word table generation complete, your 1337ness.\n");
    system("say Word table generation complete, your 1337ness.");
    
    char talkBuf[1024];
    char checkTypeFormat[] = "SELECT *, COUNT(*) FROM 'WORDS' WHERE (type = %d)";
    int typeConsts[] = {(int)'v', (int)'A', (int)'N', (int)'V', (int)'t', (int)'i'};
    for (int i = 0; i<6; i++){
        sprintf(commandBuffer, checkTypeFormat, typeConsts[i]);
        struct LinkedList* response = databaseSelect(commandBuffer, db, 1);
        //printf("Ran query %s\n", commandBuffer);
        int count = intColumn(response->value, 3);
        printf("There are %d %c words.\n", count, (char) typeConsts[i]);
        //sprintf(talkBuf, "say There are %d \"%c\" words.", count, (char) typeConsts[i]);
        //system(talkBuf);
        freeRows(response);
    }
    char checkVerbs[] = "SELECT *, COUNT(*) FROM 'WORDS' WHERE (type IN (116, 86, 105))";
    
    struct LinkedList* response = databaseSelect(checkVerbs, db, 1);
    //printf("Ran query %s\n", commandBuffer);
    int count = intColumn(response->value, 3);
    printf("There are %d total verbs.\n", count);
    //sprintf(talkBuf, "say There are %d total verbs.", count);
    //system(talkBuf);
    freeRows(response);
    fclose(ifp);
    
    //system("say There are over 9000 words! You are so 1337.");
    *longestWord = longest;
    return RET_NO_ERROR;
}






