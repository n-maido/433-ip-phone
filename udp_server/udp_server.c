#include <stdio.h>
#include <math.h>           // for ceil()
#include <ctype.h>          //for tolower()
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>

#include "udp_server.h"

#define PORT 11037
//predefined length in assignment spec for UDP packet
#define MSG_LEN 100
#define MAX_COMMAND_LENGTH 8
#define MAX_REPLY_SIZE 150
#define MAX_SIP_ADDRESS_SIZE 50

static pthread_t udpThreadPID = -1;
static struct sockaddr_in beagle_sin = {0};
static struct sockaddr_in remote_sin = {0};
static uint32_t sin_len = sizeof(remote_sin);
static int32_t udpSocket = 0;
static bool waitForMessages = false;
static pthread_cond_t * shutdownRequest = NULL;
static pthread_mutex_t * shutdownLock = NULL;

typedef enum {
    GET_CALL_STATS = 0,
    MAKE_CALL,
    END_CALL,
    STOP,
    OPTIONS_MAX_COUNT
} UDP_OPTIONS;

const char optionValues[OPTIONS_MAX_COUNT][24] = {"get_call_stats", "make_call=", "end_call=", "stop"};

static inline void toLowerString(char * s){
    for(int i = 0; i < strlen(s); ++i){
        s[i] = tolower(s[i]);
    }
}

static void sendShutdownRequest(void){
    pthread_mutex_lock(shutdownLock);
    pthread_cond_signal(shutdownRequest);
    pthread_mutex_unlock(shutdownLock);
}

/**
 * Given a msg of the format "cmd=<sip address>", extracts the sip address 
 * and copies to a provided string
 * Called by the make_call and end_call cmds
*/
static void parseSip(char* msg, UDP_OPTIONS option, char sipAddress[MAX_SIP_ADDRESS_SIZE]) {
    memset(sipAddress, 0, MAX_SIP_ADDRESS_SIZE);
    int startIndex = strlen(optionValues[option]);
    int endIndex = strlen(msg) - startIndex;
    strncpy(sipAddress, msg + startIndex, endIndex);
}

/** 
 * Response messages are json objects in the format: 
 * {
 *     "msgType": "commandName",
 *     "content": "msgContent"
 * }
 */     
static void processReply(char * msg, const unsigned int msgLen, char * r){
    toLowerString(msg);
    //need to check if enter has been pressed.
    if(!strncmp(msg, optionValues[GET_CALL_STATS], strlen(optionValues[GET_CALL_STATS]))){
        snprintf(r, 100, "{\"msgType\":\"call_stats\",\"content\":{}}\n");
        return;
    } else if(!strncmp(msg, optionValues[MAKE_CALL], strlen(optionValues[MAKE_CALL]))){
        // expects a msg of the format "make_call=<sip address>"

        // parse sip address
        char calleeAddress[MAX_SIP_ADDRESS_SIZE] = "";
        parseSip(msg, MAKE_CALL, calleeAddress);

        printf("starting call with address %s\n", calleeAddress);

        // TODO: check if valid sip address?
        bool validAdress = true; // placeholder, remove later
        if (!validAdress){
            strncpy(r, "{\"msgType\":\"make_call\", \"content\": \"Error: Invalid SIP address.\"}\n", 100);
            return;
        }else{
            // TODO: start call
            snprintf(r, MAX_REPLY_SIZE, "{\"msgType\":\"make_call\", \"content\": \"Success: Starting call to %s\"}\n", calleeAddress);
        }
    } else if (!strncmp(msg, optionValues[END_CALL], strlen(optionValues[END_CALL]))){
        // expects a msg of the format "end_call=<sip address>"
        // if we don't need an address to end a call, then delete this section

        // parse sip address
        char calleeAddress[MAX_SIP_ADDRESS_SIZE] = "";
        parseSip(msg, END_CALL, calleeAddress);

        printf("ending call with address %s\n", calleeAddress);

        // TODO: check if valid sip address?
        bool validAdress = true; // placeholder, remove later
        if (!validAdress){
            strncpy(r, "{\"msgType\":\"end_call\", \"content\": \"Error: Invalid SIP address.\"}\n", 100);
            return;
        }else{
            // TODO: end call
            snprintf(r, MAX_REPLY_SIZE, "{\"msgType\":\"end_call\", \"content\": \"Success: Ending call with %s\"}\n", calleeAddress);
        }
       
    } else if (!strncmp(msg, optionValues[STOP], strlen(optionValues[STOP]))){
        strncpy(r, "{\"msgType\":\"stop\", \"content\": \"Stopping program.\"}\n", 100);
        return;
    } else {
        strncpy(r, "{\"msgType\":\"invalid\", \"content\":\"Error: Invalid command.\"}\n", 100);
        return;
    } 
}


//constantly wait for messages on the given port. 
//returns -1 on failure, and will send out a shutdown request if it fails / receives a quit request.
static int udp_receive_thread(void){
    printf("Listening on UDP port 11037.\n");
    while(waitForMessages){
        // struct sockaddr_in sinRemote = {0};
        char messageRx[MSG_LEN] = "";
        int32_t bytesRx = recvfrom(udpSocket, messageRx, MSG_LEN - 1, 0, (struct sockaddr*) &remote_sin, &sin_len);
        if(bytesRx < 0){
            perror("Error in recvfrom");
            sendShutdownRequest();
            return -1;
        }
        messageRx[bytesRx] = 0;
        // printf("Message = %s\n", messageRx);

        // printf("%s transmitted\n", messageTx);
        
        //generate and send off the reply. 
        char reply[MAX_REPLY_SIZE] = "";
        memset(reply, 0, MAX_REPLY_SIZE);

        processReply(messageRx, bytesRx, reply);

        sendto(udpSocket, reply, strlen(reply), 0, (struct sockaddr*) &remote_sin, sin_len);
        memset(reply, 0, MAX_REPLY_SIZE);

        if(!strncmp(messageRx, optionValues[STOP], strlen(optionValues[STOP]))){
            waitForMessages = false;
            sendShutdownRequest();
            sendto(udpSocket, "", strlen(""), 0, (struct sockaddr*) &remote_sin, sin_len);
            break;
        }
        
    }
    return 0;
}

int udp_init(pthread_cond_t * cond, pthread_mutex_t * lock){
    shutdownRequest = cond;
    shutdownLock = lock;
    // memset(&beagle_sin, 0, sizeof(struct sockaddr_in));
    beagle_sin.sin_family = AF_INET;
    beagle_sin.sin_addr.s_addr = htonl(INADDR_ANY);
    beagle_sin.sin_port = htons(PORT);

    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
    
    if(!udpSocket){
        perror("Socket creation failed");
        sendShutdownRequest();
        return -1;
    }

    //binding
    if(bind(udpSocket, (struct sockaddr*) &beagle_sin, sizeof(beagle_sin))){
        perror("Bind failed");
        if(close(udpSocket) == -1){
            perror("Closing socket descripter failed after failed bind.");
        }
        sendShutdownRequest();
        return -1;
    }

    waitForMessages = true;
    if(pthread_create(&udpThreadPID, NULL, (void*)&udp_receive_thread, NULL) != 0){
        perror("Error in creating udp receiving thread.");
        sendShutdownRequest();
        return -1;
    }

    return 1;
}

int udp_cleanup(void){
    //kill thread and close socket

    //Sending the quit message myself to allow for graceful shutdown rather than forced
    //thread killing.
    printf("Sending stop signal:\n");
    char quit[5] = "stop";
    sendto(udpSocket, quit, strlen(quit), 0, (struct sockaddr *) &beagle_sin, sin_len);
    if(pthread_join(udpThreadPID, NULL) != 0){
        perror("Error in joining the UDP thread.");
    }
    close(udpSocket);
    return 1;
}