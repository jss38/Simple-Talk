#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "list.h"

static char* myPort;
static char* theirMachine;
static char* theirPort;

static List* outputList;
static List* inputList;

// Socket for output
static int sockfd1;
static struct addrinfo *servinfo1;

// Socket for input
static int sockfd2;
static struct addrinfo *servinfo2;

static struct sockaddr_storage their_addr;
static socklen_t addr_len;

static pthread_t kbdThread;
static pthread_t outputThread;
static pthread_t inputThread;
static pthread_t scrnThread;

static pthread_cond_t s_syncKbdAndOutputCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncKbdAndOutputMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t s_syncInputAndScrnCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncInputAndScrnMutex = PTHREAD_MUTEX_INITIALIZER;

// gcc -pthread s-talk.c list.c -o s-talk
// ./s-talk 4020 asb9840u-b07 4021
// ./s-talk 4021 asb9840u-b07 4020
// Use different sockets everytime

// pthread_mutex_lock(&putMutex);
// pthread_mutex_unlock(&putMutex);

void* kbdFn() {
    while(1) {
        char msg[100];
        fgets(msg, 100, stdin);
        List_append(outputList, msg);

        pthread_mutex_lock(&s_syncKbdAndOutputMutex);
        {
            pthread_cond_signal(&s_syncKbdAndOutputCondVar);
        }
        pthread_mutex_unlock(&s_syncKbdAndOutputMutex);
    }
}

void* scrnFn() {
    while(1) {
        pthread_mutex_lock(&s_syncInputAndScrnMutex);
        {
            pthread_cond_wait(&s_syncInputAndScrnCondVar, &s_syncInputAndScrnMutex);
        }
        pthread_mutex_unlock(&s_syncInputAndScrnMutex);

        if (List_count(inputList) > 0){
            List_first(inputList);
            char* msg = (char*) List_remove(inputList);
            fputs(msg, stdout);
        }
    }
}


void* outputFn() {
    struct addrinfo hints, *i;

    memset(&hints, 0, sizeof(hints));
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(theirMachine, theirPort, &hints, &servinfo1);

    for(i = servinfo1; i != NULL; i = i->ai_next) {
        sockfd1 = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
    
        if (sockfd1 == -1)
        {
            perror("Listener socket error");
            continue;
        }

        break;
    }

    while(1) {
        pthread_mutex_lock(&s_syncKbdAndOutputMutex);
        {
            pthread_cond_wait(&s_syncKbdAndOutputCondVar, &s_syncKbdAndOutputMutex);
        }
        pthread_mutex_unlock(&s_syncKbdAndOutputMutex);

        if(List_count(outputList) > 0) {
            List_first(outputList);
            char* msg = (char*) List_remove(outputList);

            int sendCheck = sendto(sockfd1, msg, 100, 0, servinfo1->ai_addr, servinfo1->ai_addrlen);
            if (sendCheck == -1){
                perror("Error with sending");
                exit(-1);
            }
            if (strcmp(msg,"!\n")==0){
                shutdown(sockfd2,SHUT_RDWR);
                shutdown(sockfd1,SHUT_RDWR);
            }
        }
    }
}

void* inputFn() {
    struct addrinfo hints,*i;

    memset(&hints, 0, sizeof(hints));
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, myPort, &hints, &servinfo2);

    for(i = servinfo2; i != NULL; i = i->ai_next) {
        sockfd2 = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
    
        if (sockfd2 == -1) {
            perror("Listener socket error");
            continue;
        }
        
        int bindTrue = bind(sockfd2, servinfo2->ai_addr, servinfo2->ai_addrlen);
        
        if (bindTrue == -1) {
        	perror("Listener bind error");	
        	continue;
        }

        break;
    }

    addr_len = sizeof(their_addr);
    
    while(1) { 
        char str[100];
        int receiveCheck = recvfrom(sockfd2, str, 100, 0, (struct sockaddr*) &their_addr, &addr_len);
        if (strcmp(str,"!\n")==0){
            shutdown(sockfd2,SHUT_RDWR);
            shutdown(sockfd1,SHUT_RDWR);
        }
        if (receiveCheck == -1 || receiveCheck == 0){
            perror("receive failed");
            exit(-1);
        }

        List_append(inputList, str);

        pthread_mutex_lock(&s_syncInputAndScrnMutex);
        {
            pthread_cond_signal(&s_syncInputAndScrnCondVar);
        }
        pthread_mutex_unlock(&s_syncInputAndScrnMutex);
    }
}

void outputThreadInit() {
    int pthreadCheck = pthread_create(&outputThread, NULL, outputFn, NULL);
    if (pthreadCheck != 0){
        perror("Output thread error");
        exit(-1);
    }
}

void outputThreadShutdown() {
    pthread_join(outputThread, NULL);
}

void inputThreadInit() {
    int inputThreadCheck = pthread_create(&inputThread, NULL, inputFn, NULL);
    if (inputThreadCheck != 0){
        perror("Input thread error");
        exit(-1);
    }
}

void inputThreadShutdown() {
    pthread_join(inputThread, NULL);
}

void kbdThreadInit() {
    int kbdThreadCheck = pthread_create(&kbdThread, NULL, kbdFn, NULL);
    if (kbdThreadCheck != 0){
        perror("Kbd thread error");
        exit(-1);
    }
}

void kbdThreadShutdown() {
    pthread_join(kbdThread, NULL);
}

void scrnThreadInit() {
    int scrnThreadCheck = pthread_create(&scrnThread, NULL, scrnFn, NULL);
    if (scrnThreadCheck != 0){
        perror("Scrn thread error");
        exit(-1);
    }
}

void scrnThreadShutdown() {
    pthread_join(scrnThread, NULL);
}


int main(int argc, char *argv[]) {
    myPort = argv[1];
    theirMachine = argv[2];
    theirPort = argv[3];

    outputList = List_create();
    inputList = List_create();

    outputThreadInit();
    kbdThreadInit();
    scrnThreadInit();
    inputThreadInit();

    outputThreadShutdown();
    kbdThreadShutdown();
    scrnThreadShutdown();
    inputThreadShutdown();

    return 0;
}
