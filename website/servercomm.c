#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

size_t cTalkSend(int fd, unsigned char* message, size_t messageLen){
    int headerLen = (int)(log10(messageLen)/log10(128)) + 1;
    uint8_t header[headerLen];
    size_t tempLen = messageLen;
    for (int i = 0; tempLen>0; i++){
        header[i] = tempLen%128;
        tempLen /= 128;
        if (tempLen>0) header[i] &= (1 << 7);
    }
    struct iovec iov[2];
    iov[0].iov_base = header;
    iov[0].iov_len = headerLen;
    iov[1].iov_base = message;
    iov[1].iov_len = messageLen;
    size_t send;
    send = writev(fd, iov, 2);
    if (send==-1) return 0;
    return messageLen;
}

size_t cTalkRecv(int fd, unsigned char* buffer, size_t bufferLen){
    long recvLength = 0;
    size_t length = 0;
    bool end = false;
    int i = 0;
    while (!end){
        recvLength = read(fd, buffer, sizeof(unsigned char));
        if (recvLength<=0) return 0;
        end = ((buffer[0] >> 7) == 0);
        if (!end) buffer[0] &= ~(1 << 7);
        length+=buffer[0]*powl(128, i);
        if (length>bufferLen) return 0;
        i++;
    }
    size_t bytesRead = 0;
    while (bytesRead < length){
        recvLength = read(fd, buffer + bytesRead, length-bytesRead);
        if (recvLength<=0) return 0;
        bytesRead+=recvLength;
    }
    return bytesRead;
}

#define alert(str) \
do{ \
system("say alert"); \
system("say \
str \
"); \
} while (0); \

int main(){
    //system("say CGI script being loaded...");
    int sockfd = 0;
    printf("%s%c%c\n",
           "Content-Type:text/html;charset=iso-8859-1",13,10);
    printf("<TITLE>BrickDNS API</TITLE>\n");
    char* data = getenv("QUERY_STRING");
    if(data == NULL){
        //system("say check");
        printf("Error in passing data from form to script.\n");
        alert("Error");
        //system("say Error in passing data from form to script.");
    }
    struct sockaddr_in serv_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Error : Could not create socket \n");
        alert("Error");
        //system("say Error: could not create socket.");
        return 1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9001);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){
        printf("\n inet_pton error occured\n");
        alert("Error");
        return 1;
    }
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("\n Error : Connect Failed \n");
        alert("Error");
        return 1;
    }
    char inBuf[512];
    switch (data[0]){
        case '1':{
            cTalkSend(sockfd, data, strlen(data)+1);
            cTalkRecv(sockfd, inBuf, 512);
            char url[512];
            char* token = NULL; char *state;
            token = strtok_r(inBuf, ",", &state);
            if (!token){ printf("error"); return 1; }
            float latitude = atof(token)/100000;
            token = strtok_r(NULL, ",", &state);
            if (!token){ printf("error"); return 1; }
            float longitude = atof(token)/100000;
            sprintf(url, "<a href=\"https://www.google.com/maps/@%f,%f,20z\">Google Maps</a>", latitude, longitude);
            printf("%f, %f<br><br>%s", latitude, longitude, url);
            //printf("%s", url);
            break;
        }
        case '2':{
            char* token; char *state;
            token = strtok_r(data+sizeof(char), ",", &state);
            if (!token){ printf("error"); return 1; }
            float latitude = atof(token);
            token = strtok_r(NULL, ",", &state);
            if (!token){ printf("error"); return 1; }
            float longitude = atof(token);
            char alteredMessage[strlen(data)+10];
            sprintf(alteredMessage, "2%d,%d", (int)(latitude*100000), (int)(longitude*100000));
            //printf("%s", alteredMessage);
            cTalkSend(sockfd, alteredMessage, strlen(alteredMessage)+1);
            cTalkRecv(sockfd, inBuf, 512);
            printf("%s\n\n", inBuf);
            break;
        }
        default:{
            printf("Invalid input.\n");
            alert("Error");
            return 1;
        }
    }
    return 0;
}