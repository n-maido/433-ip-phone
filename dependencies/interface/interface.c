#include "../joystick/joystick.h"
#include "../LCD/lcd.h"
#include "../pot/pot.h"
#include "../utils/util.h"
#include "interface.h"
#include "../../module_pjsua/pjsua_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <pjsua-lib/pjsua.h>

//Linked list of users
struct IFace_user {
    char* name;
    char* sip;
    struct IFace_user *prev;
    struct IFace_user *next;
};

static struct IFace_user *IFace_currentUser;
static struct IFace_user *IFace_lastUser;
static struct IFace_user *IFace_firstUser;
static bool IFace_running = false;

static pthread_t tid;
static pthread_attr_t attr;

//0=no call in session , 1 = incoming call ringing, 2= call in session, 3=outgoing call
enum IFace_status {
    NOCALL,
    INCOMING,
    INCALL,
    OUTGOING,
    ERROR
};

static enum IFace_status currentStatus = NOCALL;

static struct IFace_user* IFace_createUser(char* username, char* sip);

static char *address;

/*int main(){
    IFace_initialize();
    IFace_addUser("test1", "1");
    IFace_addUser("test2", "2");
    IFace_addUser("test3", "3");
    sleepMs(10000);
    IFace_cleanup();
}*/

void IFace_initialize() {
    //create thread, create default user and initialize current / last user, initialize joystick and lcd
    JS_initialize();
    LCD_initialize();
    IFace_currentUser = IFace_createUser("   Main Menu   ", "              ->");
    IFace_lastUser = IFace_currentUser;
    IFace_firstUser = IFace_currentUser;
    LCD_writeMessage(IFace_currentUser->name, IFace_currentUser->sip);

    IFace_addUser("Ryan's BBG", "sip:beagle@192.168.1.207");
    IFace_addUser("Misha's BBG", "sip:beagle@192.168.1.58");
    IFace_addUser("Misha's Android", "sip:misha@192.168.1.82:57440");
    IFace_addUser("San's BBG", "sip:beagle@192.168.1.129");
    IFace_addUser("Nhi's BBG", "sip:beagle@192.168.1.5");

    address = (char*)malloc(CURRENT_URI_SIZE);
    
    IFace_running = true;
    //pthread_attr_init(&attr);
    //pthread_create(&tid, &attr, IFace_runner, NULL);
}

static struct IFace_user* IFace_createUser(char* username, char* sip){
    struct IFace_user *newUser = (struct IFace_user*)malloc(sizeof(struct IFace_user));

    char* user_name = (char*)malloc(sizeof(char) * strlen(username) + 1);
    char* user_sip = (char*)malloc(sizeof(char) * strlen(sip) + 1);

    newUser->name = strcpy(user_name, username);
    newUser->sip = strcpy(user_sip, sip);
    newUser->next = NULL;
    newUser->prev = NULL;

    return newUser;
}
static struct IFace_user* IFace_findUser(char* sip) {
        struct IFace_user *curr = IFace_lastUser;
    while (strcmp(curr->sip, sip) != 0) {
        if (curr->prev == NULL) {
            return NULL;
        }
        curr = curr->prev;
    }
    return curr;
}


void IFace_addUser(char* username, char* sip) {
    if (IFace_currentUser == NULL || IFace_lastUser == NULL) {
        printf("\nERROR: INTERFACE NOT INITIALIZED!\n");
        return;
    }

    struct IFace_user *newUser = IFace_createUser(username, sip);
     
    if (IFace_findUser(sip) != NULL) {
        printf("\nERROR: Tried to add a user into interface that already exists.\n");
        free(newUser->name);
        free(newUser->sip);
        free(newUser);
        return;
    }

    IFace_lastUser->next = newUser;
    newUser->prev = IFace_lastUser;
    IFace_lastUser = newUser;
}

void IFace_removeUser(char* sip){
    if (IFace_currentUser == NULL || IFace_lastUser == NULL) {
        printf("\nERROR: INTERFACE NOT INITIALIZED!\n");
        return;
    }

    struct IFace_user *userToDelete = IFace_findUser(sip);
        if (userToDelete == NULL) {
            printf("ERROR: IFace_removeUser(\"%s\") could not find that user to remove\n", sip);
            return;
        }

    //Update screen if current user is displayed
    if (strcmp(sip, IFace_currentUser->sip) == 0) {
        IFace_currentUser = IFace_currentUser->prev;
        LCD_writeMessage(IFace_currentUser->name, IFace_currentUser->sip);
    }

    if (userToDelete->next != NULL){
        userToDelete->next->prev = userToDelete->prev;
    }
    if (userToDelete->prev != NULL){
        userToDelete->prev->next = userToDelete->next;
    }

    free(userToDelete->name);
    free(userToDelete->sip);
    free(userToDelete);
}

static char* IFace_getName(char* sip) {
    struct IFace_user *user = IFace_findUser(sip);
    if (user == NULL) return sip;
    return user->name;
}

static char* IFace_extractIp(char* str) {
    for (int i = 0; i < strlen(str); i++){
        if (str[i] == '@') return &str[i + 1];
    }
    return str;
}

void IFace_updateStatus() {
    if (IFace_currentUser == NULL || IFace_lastUser == NULL) {
        printf("\nERROR: INTERFACE NOT INITIALIZED!\n");
        return;
    }

    int status = pjsua_interface_get_status_call();

    if (status != currentStatus){
        pjsua_interface_get_uri_alt(address);
        switch (status){
            case NOCALL:
            IFace_currentUser = IFace_firstUser;
            LCD_writeMessage(IFace_currentUser->name, IFace_currentUser->sip);
            break;
            case INCOMING:
            LCD_writeMessage("Call from:", IFace_extractIp(IFace_getName(address)));
            break;
            case INCALL:
            LCD_writeMessage("In call with:", IFace_extractIp(IFace_getName(address)));
            break;
            case OUTGOING:
            LCD_writeMessage("Call outgoing...", IFace_extractIp(IFace_getName(address)));
            break;
            default:
            break;
        }


        currentStatus = status;
    }
}

int abs (int n){
    return n + 2 * n * (n < 0);
}

void* IFace_runner(void* arg) {
    if (IFace_currentUser == NULL || IFace_lastUser == NULL) {
        printf("\nERROR: INTERFACE NOT INITIALIZED!\n");
        return NULL;
    }
    
    //main interface loop
    while(IFace_running){
        enum JS_direction input = JS_read();

        int lastVolume = -1;

        //Await input
        while (input == NONE && IFace_running) {
            sleepMs(100);
            IFace_updateStatus();
            int volume = POT_readValue();
            if (abs(volume - lastVolume) > 2){
                pjsua_interface_set_volume(volume);
                lastVolume = volume;
            }
            input = JS_read();
        }

        //Pick up or decline
        if (currentStatus == INCOMING){
            if (input == IN){
                pjsua_interface_pickup_incoming_call(1);
            } else if (input == DOWN) {
                pjsua_interface_pickup_incoming_call(2);
            }
        }

        //Hang up
        if (currentStatus == OUTGOING || currentStatus == INCALL && input == DOWN) {
            pjsua_interface_hang_up_call();
        }

        //Menu
        if (currentStatus != NOCALL) continue;
        switch (input){
            //Scroll back a contact
            case LEFT:
                if (IFace_currentUser->prev != NULL){
                    IFace_currentUser = IFace_currentUser->prev;
                    LCD_writeMessage(IFace_currentUser->name, IFace_extractIp(IFace_currentUser->sip));
                }
                break;
            //Scroll to next contact
            case RIGHT:
                if (IFace_currentUser->next != NULL){
                    IFace_currentUser = IFace_currentUser->next;
                    LCD_writeMessage(IFace_currentUser->name, IFace_extractIp(IFace_currentUser->sip));
                }
                break;
            //Start call
            case IN:
                if (IFace_currentUser->prev != NULL){
                    printf("Calling user '%s' at %s\n", IFace_currentUser->name, IFace_currentUser->sip);
                    int returnStatus = pjsua_interface_make_call(IFace_currentUser->sip);
                    
                    if (returnStatus == 0) {
                        LCD_writeMessage("Error Calling", "Try again later");
                    } else {
                        LCD_writeMessage("User may be busy", "");
                    }
                }
                break;
            default:
            break;
        }
    }

    return NULL;
}

void IFace_endThread() {
    IFace_running = false;
}

void IFace_cleanup() {
    if (IFace_currentUser == NULL || IFace_lastUser == NULL) {
        printf("\nERROR: INTERFACE NOT INITIALIZED!\n");
        return;
    }

    //remove thread, delete all users
    IFace_running = false;

    //pthread_join(*threadID, NULL);

    struct IFace_user *next = IFace_lastUser->prev;
    for(IFace_currentUser = IFace_lastUser; IFace_currentUser != NULL; IFace_currentUser = next){
        free(IFace_currentUser->name);
        free(IFace_currentUser->sip);
        next = IFace_currentUser->prev;
        free(IFace_currentUser);
    }

    free(address);
}