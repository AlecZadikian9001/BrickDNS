//
//  Server.c
//  BrickDNS
//
//  Created by Alec Zadikian on 9/11/14.
//  Copyright (c) 2014 AlecZ. All rights reserved.
//

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

/* Constants for coordinate-number conversion: */
#define MIN_LATITIUDE (-90)
#define MAX_LATITUDE (90)
#define MIN_LONGITUDE (-180.00000)
#define MAX_LONGITUDE (180.00000)
// add more if you need...
/* */

// Should map coordinates to a number with a one-to-one relationship.
// Coordinates are given as standard lat/long with precision down to hundred thousandths.
// Latitude goes from -90.00000˚ to 90.00000˚ (inclusive)
// Longitude goes from -180.00000˚ to 179.99999˚ (inclusive)
// Output has to be uint64_t (unsigned 64-bit integer) to have a big enough co-domain. Do NOT convert the uint64_t to anything else.
uint64_t numberFromCoordinates(float latitude, float longitude){
    return 0; // rewrite this
}

//Undoes the above function, converting num to latitude and longitude. Leaves latitude and longitude pointing to the results.
void coordinatesFromNumber(uint64_t num, float* latitude, float* longitude){
    
  //Set the latitude and longitude pointers to whatever they should be like this:
  //  *latitude = your_latitude_output;
  //  *longitude = your_longitude_output;
  //
}

struct ServerWorkerThread{
    struct Server* server; // the server options
    int threadID; // the thread ID for this worker thread
};

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
                cTalkSend(clntSock, "echo", strlen("echo")+1); // TODO replace with actual code
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