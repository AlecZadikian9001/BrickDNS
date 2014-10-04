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
    char createWordTable[] = "CREATE TABLE 'WORDS' ('word' char(1024) PRIMARY KEY UNIQUE NOT NULL, 'type1' int(3) NOT NULL, 'type2' int(3) NOT NULL, 'type3' int(3) NOT NULL, 'type4' int(3) NOT NULL, 'type5' int(3) NOT NULL)";
    databaseCommand(createWordTable, db);
    char addToTableFormat[] = "INSERT INTO 'WORDS' ('word', 'type1', 'type2', 'type3', 'type4', 'type5') VALUES ('%s' , '%d', '%d', '%d', '%d', '%d')";
    char commandBuffer[1024];
    FILE *ifp;
    char *mode = "r";
    ifp = fopen("dictionary.txt", mode);
    if (ifp == NULL) {
        fprintf(stderr, "Can't open input file in.list!\n");
        return RET_INPUT_ERROR;
    }
    char wordBuffer[128];
    int bigIndex = 0;
    int type[5];
    int length;
    while (fscanf(ifp, "%s\r", wordBuffer) != EOF) {
        length = strlen(wordBuffer);
        for (int i = 0; i<strlen(wordBuffer); i++){
            if (wordBuffer[i]==' ' || (wordBuffer[i]>='A' && wordBuffer[i]<='Z')) break;
            if (wordBuffer[i]=='\xd7'){
                wordBuffer[i] = '\0';
                i++;
                bzero(type, sizeof(type));
                for (int i2 = 0; i2 < 5; i2++){
                    if (i+i2 >= length) break;
                    type[i2] = wordBuffer[i+i2];
                }
                sprintf(commandBuffer, addToTableFormat, wordBuffer, type[0], type[1], type[2], type[3], type[4]);
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
    char checkTypeFormat[] = "SELECT *, COUNT(*) FROM 'WORDS' WHERE ((type1 = %d AND type1 != 0) OR (type2 = %d AND type2 != 0) OR (type3 = %d AND type3 != 0) OR (type4 = %d AND type4 != 0) OR (type5 = %d AND type5 !=0))";
    int typeConsts[] = {(int)'v', (int)'A', (int)'N', (int)'V', (int)'t', (int)'i'};
    for (int i = 0; i<6; i++){
        sprintf(commandBuffer, checkTypeFormat, typeConsts[i], typeConsts[i], typeConsts[i], typeConsts[i]);
        struct LinkedList* response = databaseSelect(commandBuffer, db, 1);
        //printf("Ran query %s\n", commandBuffer);
        int count = intColumn(response->value, 6);
        printf("There are %d %c words.\n", count, (char) typeConsts[i]);
        sprintf(talkBuf, "say There are %d \"%c\" words.", count, (char) typeConsts[i]);
        system(talkBuf);
        freeRows(response);
    }
    char checkVerbs[] = "SELECT *, COUNT(*) FROM 'WORDS' WHERE ((type1 IN (116, 86, 105)  AND type1 != 0) OR (type2 IN (116, 86, 105) AND type2 != 0) OR (type3 IN (116, 86, 105) AND type3 != 0) OR (type4 IN (116, 86, 105) AND type4 != 0) OR (type5 IN (116, 86, 105) AND type5 !=0))";
    
    struct LinkedList* response = databaseSelect(checkVerbs, db, 1);
    //printf("Ran query %s\n", commandBuffer);
    int count = intColumn(response->value, 6);
    printf("There are %d total verbs (ignoring overlaps).\n", count);
    sprintf(talkBuf, "say There are %d total verbs ignoring overlaps.", count);
    system(talkBuf);
    freeRows(response);
    
    system("say There are over 9000 words! You are so 1337.");
    
    return RET_NO_ERROR;
}






