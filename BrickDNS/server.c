//
//  Server.c
//  BrickDNS
//
//  Created by Alec Zadikian on 9/11/14.
//  Copyright (c) 2014 AlecZ. All rights reserved.
//

/*
 ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗     ██████╗
 ██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗   ██╔════╝
 ███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝   ██║
 ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗   ██║
 ███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║██╗╚██████╗
 ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝
 */

#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include "general.h"
#include "ctalk.h"
#include "server.h"

/* Constants for English language: */
#define LONGEST_WORD 27
#define MAX_REPLY_LENGTH 1024 // TODO fix
/* */

/* Constants for coordinate-number conversion: */
#define MIN_LATITIUDE (-9000000)
#define MAX_LATITUDE (9000000)
#define MIN_LONGITUDE (-18000000)
#define MAX_LONGITUDE (18000000)
// add more if you need, Arman...
/* */

// *** Arman, you can do this one if you want. *** //
// Should map coordinates to a number with a one-to-one relationship.
// Coordinates are given as standard lat/long with precision down to hundred thousandths.
// Latitude goes from -9000000 to 9000000 (inclusive)
// Longitude goes from -18000000˚ to 17999999 (inclusive)
// Output has to be uint64_t (unsigned 64-bit integer) to have a big enough co-domain. Do NOT convert the uint64_t to anything else.
uint64_t numberFromCoordinates(int latitude, int longitude){
    uint64_t number;
    latitude = latitude + 90000000;
    longitude = longitude + 18000000;
    number = longitude + (latitude*35999999);
    return number;
}

// *** Arman, you can do this one if you want. *** //
//Undoes the above function, converting num to latitude and longitude. Leaves latitude and longitude pointing to the results.
void coordinatesFromNumber(uint64_t number, int* latitude, int* longitude){
    //TODO
  //Set the latitude and longitude pointers to whatever they should be like this:
  //  *latitude = your_latitude_output;
  //  *longitude = your_longitude_output;
  //
    *latitude = number % 35999999;
    *longitude = (int)number - (*latitude*35999999);
    
}

// I don't know how to do this one yet...
struct LinkedList* wordsFromNumber(uint64_t num){
    struct LinkedList* ret = emalloc(sizeof(struct LinkedList));
    // TODO
    return ret;
}

// I don't know about this either.
uint64_t numberFromWords(struct LinkedList* words){
    char* word;
    while (words){
        word = words->value;
        // TODO
        words = words->next;
    }
    return 0;
}

struct ServerWorkerThread{
    struct Server* server; // the server options
    unsigned int threadID; // the thread ID for this worker thread
    unsigned int numWords; // number of words in master dictionary // TODO may want to add separate dictionaries for nouns, verbs, adjectives, articles, etc.
    char* dictionary; // master dictionary, a 2D array of chars (numWords x LONGEST_WORD)
};

typedef enum{
    NET_NULL_START, // error
    NET_RESOLVE_NAME,
    NET_GET_NAME,
    NET_NULL_END // error
} ClientCommand;

void* workerThreadFunction(void* argVoid){
    struct ServerWorkerThread* arg = (struct ServerWorkerThread*) argVoid;
    int clntSock = 0;
    struct sockaddr_in clientAddress;
    socklen_t clntLen = sizeof(clientAddress);
    int servSock = arg->server->sockfd;
    struct timeval tv;
    tv.tv_sec = (int) arg->server->timeout; // timeout seconds
    tv.tv_usec = fmod(arg->server->timeout, 1.0)*1000000.0;  // timeout microseconds
    size_t recvBufLen = arg->server->recvSize;
    uint8_t* receiveBuffer = emalloc(sizeof(uint8_t)*recvBufLen);
    size_t receiveSize = 0;
    char coordinatesReplyFormat[] = "%d,%d";
    char wordsReplyFormat[] = "%s,%s,%s,%s";
    char replyBuffer[MAX_REPLY_LENGTH];
    //*** Additional thread initialization code goes here ***//
    //vvvv//
    
    //^^^^
    while (true){ // vvv begin client listening loop
        while (!arg->server->isListening) usleep(1000*200);
        if ((clntSock = accept(servSock, (struct sockaddr *) &clientAddress,
                               &clntLen)) >= 0){
            arg->server->threadUsage++;
            while (!arg->server->isListening) usleep(1000*200);
            setsockopt(clntSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv,sizeof(tv));
            if (logLevel >= LOG_ABNORMAL && arg->server->threadUsage >= arg->server->threadLimit) printf("Warning: All worker threads are in use. Consider creating more.\n");
            char* ipAddr = inet_ntoa(clientAddress.sin_addr);
            if (logLevel >= LOG_INFORMATIONAL) printf("Handling client %s on thread %d\n", ipAddr, arg->threadID);
            //*** Additional client connection handling code goes here ***//
            //vvvv//
            
            //^^^^
            do{
                receiveSize = cTalkRecv(clntSock, receiveBuffer, recvBufLen);
                if (!arg->server->keepAlive) shutdown(clntSock, 0); // stop receiving data
                if (receiveSize < 1 || receiveSize > recvBufLen){
                    break;
                }
                //*** Additional data from client handling code goes here ***//
                //vvvv//
                if (logLevel >= LOG_FULL) printf("Received message: %s\nof length %lu\n", receiveBuffer, receiveSize);
                ClientCommand cmd = ((int) receiveBuffer[0])+NET_NULL_START;
                if (cmd >= NET_NULL_END || cmd <= NET_NULL_START){
                    if (logLevel >= LOG_ABNORMAL) printf("Invalid command %d from %s; disconnecting.\n", cmd, ipAddr);
                    break;
                }
                receiveBuffer[recvBufLen-1] = '\0';
                switch (cmd){
                    case NET_GET_NAME:{
                        char *token;
                        char *state;
                        float latitude, longitude;
                        int i = 0;
                        for (token = strtok_r(receiveBuffer, "&", &state); token != NULL && i<2; token = strtok_r(NULL, "&", &state)){
                            if (i==0) latitude = atof(token);
                            else longitude = atof(token);
                            i++;
                        }
                        uint64_t num = numberFromCoordinates(latitude, longitude);
                        struct LinkedList* words = wordsFromNumber(num);
                        sprintf(replyBuffer, wordsReplyFormat, words->value, words->next->value, words->next->next->value, words->next->next->next->value);
                        cTalkSend(clntSock, replyBuffer, strlen(replyBuffer)+1);
                        break;
                    }
                    case NET_RESOLVE_NAME:{
                        char *token;
                        char *state;
                        struct LinkedList* head = emalloc(sizeof(struct LinkedList));
                        struct LinkedList* node = head;
                        for (token = strtok_r(receiveBuffer, "&", &state); token != NULL; token = strtok_r(NULL, "&", &state)){
                            node->value = token;
                            node->size = strlen(token)+1;
                            node = node->next;
                        }
                        uint64_t num = numberFromWords(head);
                        int latitude, longitude;
                        coordinatesFromNumber(num, &latitude, &longitude);
                        sprintf(replyBuffer, coordinatesReplyFormat, latitude, longitude);
                        cTalkSend(clntSock, replyBuffer, strlen(replyBuffer)+1);
                        break;
                    }
                }
                //^^^^
            } while (arg->server->keepAlive);
            close(clntSock);
            if (logLevel >= LOG_INFORMATIONAL) printf("Client %s disconnected on thread %d.\n", ipAddr, arg->threadID);
            arg->server->threadUsage--;
            if (arg->server->threadCount > arg->server->threadLimit){
                break;
            }
        }
        else{
            if (logLevel>=LOG_ERROR){
                printf("accept() error: %d\n", errno);
            }
        }
    } // ^^^ end client listening loop
    if (logLevel >= LOG_ABNORMAL) printf("Worker thread %d shutting down.\n", arg->threadID);
    arg->server->threadCount--;
    close(clntSock);
    efree(receiveBuffer);
    efree(arg);
    pthread_detach(pthread_self());
    return NULL;
}

int changeThreadLimit(int num, struct Server* server){
    if (logLevel >= LOG_INFORMATIONAL) printf("ChangeNumWorkerThreads called, attempting to change to %d threads.\n", num);
    if (num<=0){
        if (logLevel >= LOG_ERROR) printf("Invalid thread limit %d specified in changeNumWorkerThreads. Defaulting to 64.\n", num);
        num = 64;
    }
    bool fail = false;
    int maxThreads = (int) sysconf(_SC_THREAD_THREADS_MAX);
    if (maxThreads > 0 && num > maxThreads - 4){
        server->threadLimit = maxThreads-4;
        if (logLevel >= LOG_ERROR) printf("The thread limit specified is less than 4 fewer than the maximum supported by the OS and is therefore likely unsafe. Setting it to 4 fewer than maximum, but this could theoretically still be unsafe.\n");
        fail = true;
    }
    server->threadLimit = num;
    if (num > server->threadCount){
        int created = 0;
        for (; created < server->threadLimit - server->threadCount; created++){
            struct ServerWorkerThread* arg = emalloc(sizeof(struct ServerWorkerThread));
            arg->server = server;
            arg->threadID = server->threadCount+created;
            pthread_t clientThread;
            if (pthread_create(&clientThread, NULL, workerThreadFunction, (void*) arg) != 0){
                if (logLevel >= LOG_ERROR) printf("Unable to create the number of worker threads specified.\n");
                efree(arg);
                fail = true; break;
            }
        }
        server->threadCount = server->threadCount + created;
        if (logLevel >= LOG_ABNORMAL) printf("Created %d threads.\n", created);
    }
    else{
        if (logLevel >= LOG_ABNORMAL) printf("%d threads will be shut down as new clients connect.\n", server->threadCount - server->threadLimit);
    }
    if (logLevel >= LOG_INFORMATIONAL) printf("Set thread limit to %d.\n", server->threadLimit);
    return fail?RET_THREAD_ERROR:RET_NO_ERROR;
}

int startServer(struct Server* mainArg){
    pthread_t mainThread;
    mainArg->mainThread = &mainThread;
    mainArg->threadCount = 0;
    mainArg->threadUsage = 0;
    return changeThreadLimit(mainArg->threadLimit, mainArg);
}

int stopServer(struct Server* server){
    server->isRunning = false;
    return RET_NO_ERROR;
}
int startListening(struct Server* server){
    server->isListening = true;
    return RET_NO_ERROR;
}
int stopListening(struct Server* server){
    server->isListening = false;
    return RET_NO_ERROR;
}