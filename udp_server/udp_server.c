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
    CALL_STATUS = 0,
    NEW_USER,
    ADD_CONTACT,
    MAKE_CALL,
    END_CALL,
    PICK_UP,
    STOP,
    OPTIONS_MAX_COUNT
} UDP_OPTIONS;

const char optionValues[OPTIONS_MAX_COUNT][24] = {"call_status", "new_user=", "add_contact=", "make_call=", "end_call=", "pick_up=", "stop"};

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
 * Given a msg of the format "cmd=<string>", extracts the string 
 * and copies to a provided string
 * Called by the make_call and end_call cmds to extract the sip address
 * Called by new_user to extract username
*/
static void extractString(char* msg, UDP_OPTIONS option, char sipAddress[MAX_SIP_ADDRESS_SIZE]) {
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
    if(!strncmp(msg, optionValues[CALL_STATUS], strlen(optionValues[CALL_STATUS]))){
        // TODO: get call status from Call module
        // 0 = none, 1 = incoming, 2 = ongoing, 3 = error
        int status = 2;
        char* address = "sip@sip:123.123.12.1";

        // if ongoing call, get the current volume and mic gain level
        switch (status) {
            case 0:
                snprintf(r, 100, "{\"msgType\":\"call_status\",\"content\":{\"status\": %d}}\n", status);
                break;
            case 1:
                snprintf(r, 100, "{\"msgType\":\"call_status\",\"content\":{\"status\": %d, \"address\": \"%s\"}}\n", status, address);
                break;
            case 2: 
            {
                int vol = 50;
                int gain = 10;
                snprintf(r, 150, "{\"msgType\":\"call_status\",\"content\":{\"status\": %d, \"address\": \"%s\", \"vol\": %d, \"gain\": %d}}\n", status, address, vol, gain);
                break;
            }
            case 3:
            {
                char* error = "Error";
                snprintf(r, 150, "{\"msgType\":\"call_status\",\"content\":{\"status\": %d, \"error\": \"%s\"}}\n", status, error);
                break;
            }
            default:
                snprintf(r, 100, "{\"msgType\":\"call_status\",\"content\":{\"status\": %d}}\n", status);
        }
        return;
    } else if(!strncmp(msg, optionValues[NEW_USER], strlen(optionValues[NEW_USER]))){
        // expects a msg of the format "new_user=<username>"
        // parse username
        char username[MAX_SIP_ADDRESS_SIZE] = "";
        extractString(msg, NEW_USER, username);

        printf("new user joined: %s\n", username);

        //TODO: return their sip address?
        strncpy(r, "{\"msgType\":\"new_user\", \"content\": \"Success\"}\n", 100);        
    } else if(!strncmp(msg, optionValues[ADD_CONTACT], strlen(optionValues[ADD_CONTACT]))){
        // expects a msg of the format "add_contact=<name>&<sipAddress>"
        // parse contact
        char contact[MAX_SIP_ADDRESS_SIZE] = "";
        extractString(msg, ADD_CONTACT, contact);

        char* name = strtok(contact, "&");
        char* sipAddress = strtok(NULL, "&");

        printf("new contact: %s, %s\n", name, sipAddress);

        //TODO: call LCD_add_contact(name, sipAddress)

        //TODO: return their sip address?
        strncpy(r, "{\"msgType\":\"add_contact\", \"content\": \"Success\"}\n", 100);

    } else if(!strncmp(msg, optionValues[MAKE_CALL], strlen(optionValues[MAKE_CALL]))){
        // expects a msg of the format "make_call=<sip address>"

        // parse sip address
        char calleeAddress[MAX_SIP_ADDRESS_SIZE] = "";
        extractString(msg, MAKE_CALL, calleeAddress);

        printf("starting call with address %s\n", calleeAddress);

        // TODO: call Call module's calling func and process return val.
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
        extractString(msg, END_CALL, calleeAddress);

        printf("ending call with address %s\n", calleeAddress);
        // TODO: call Call module's end call func and process return val.

        // TODO: check if valid sip address?
        bool validAdress = true; // placeholder, remove later
        if (!validAdress){
            strncpy(r, "{\"msgType\":\"end_call\", \"content\": \"Error: Invalid SIP address.\"}\n", 100);
            return;
        }else{
            // TODO: end call
            snprintf(r, MAX_REPLY_SIZE, "{\"msgType\":\"end_call\", \"content\": \"Success: Ending call with %s\"}\n", calleeAddress);
        }
       
    } else if (!strncmp(msg, optionValues[PICK_UP], strlen(optionValues[PICK_UP]))){
        // expects a msg of the format "pick_up=<sip address>"
        // if we don't need an address to end a call, then delete this section

        // parse sip address
        char calleeAddress[MAX_SIP_ADDRESS_SIZE] = "";
        extractString(msg, PICK_UP, calleeAddress);

        printf("picking up call with address %s\n", calleeAddress);
        // TODO: call Call module's pick upl func and process return val.

        bool success = true; // placeholder, remove later
        if (success){
            strncpy(r, "{\"msgType\":\"pick_up\", \"content\": \"Success\"}\n", 100);
            return;
        }else{
            snprintf(r, MAX_REPLY_SIZE, "{\"msgType\":\"pick_up\", \"content\": \"Error\"}\n");
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