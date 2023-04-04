#include "../joystick/joystick.h"
#include "../LCD/lcd.h"
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

static enum IFace_status currentStatus = NONE;

static struct IFace_user* IFace_createUser(char* username, char* sip);
static void* IFace_runner(void* arg);

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
    IFace_currentUser = IFace_createUser("Scroll <- or ->", "Push in to call");
    IFace_lastUser = IFace_currentUser;
    LCD_writeMessage(IFace_currentUser->name, IFace_currentUser->sip);

    IFace_running = true;
    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, IFace_runner, NULL);
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

void IFace_addUser(char* username, char* sip){
    if (IFace_currentUser == NULL || IFace_lastUser == NULL) {
        printf("\nERROR: INTERFACE NOT INITIALIZED!\n");
        return;
    }

    struct IFace_user *newUser = IFace_createUser(username, sip);
    IFace_lastUser->next = newUser;
    newUser->prev = IFace_lastUser;
    IFace_lastUser = newUser;
}

void IFace_removeUser(char* sip){
    if (IFace_currentUser == NULL || IFace_lastUser == NULL) {
        printf("\nERROR: INTERFACE NOT INITIALIZED!\n");
        return;
    }

    struct IFace_user *userToDelete = IFace_lastUser;
    while (strcmp(userToDelete->sip, sip) != 0) {
        userToDelete = userToDelete->prev;
    }

    //Update screen if current user is displayed
    if (strcmp(sip, IFace_currentUser->sip) == 0) {
        IFace_currentUser = IFace_currentUser->prev;
        LCD_writeMessage(IFace_currentUser->name, IFace_currentUser->sip);
    }

    userToDelete->next->prev = userToDelete->prev;
    userToDelete->prev->next = userToDelete->next;

    free(userToDelete->name);
    free(userToDelete->sip);
    free(userToDelete);
}

void IFace_updateStatus(int status, char* address) {
    if (status != currentStatus){
        switch (status){
            case NOCALL:
            //LCD_writeMessage(IFace_currentUser->name, IFace_currentUser->sip);
            break;
            case INCOMING:
            LCD_writeMessage("Call from:", address);
            break;
            case INCALL:
            LCD_writeMessage("In call with:", address);
            break;
            case OUTGOING:
            LCD_writeMessage("Ringing...", address);
            break;
            default:
            break;
        }


        currentStatus = status;
    }
}

static void* IFace_runner(void* arg) {
    /*pj_thread_t *thread = pj_thread_this();
    pj_thread_desc desc;
    pj_thread_get_desc(thread, &desc);
    status = pj_thread_register(NULL, desc, NULL);
    if (status != PJ_SUCCESS) {
        // Handle error
        pjsua_destroy();
        return 1;
    }*/
    
    //main interface loop
    while(IFace_running){
        enum JS_direction input = JS_read();
        while (input == NONE && IFace_running) {
            input = JS_read();
        }
        if (currentStatus == INCOMING){
            if (input == IN){
                pjsua_interface_pickup_incoming_call(1);
            } else if (input == DOWN) {
                pjsua_interface_pickup_incoming_call(2);
            }
        }
        if (currentStatus == OUTGOING && input == DOWN) {
            pjsua_interface_hang_up_call();
        }
        if (currentStatus != NOCALL) continue;
        switch (input){
            //Scroll back a contact
            case LEFT:
                if (IFace_currentUser->prev != NULL){
                    IFace_currentUser = IFace_currentUser->prev;
                    LCD_writeMessage(IFace_currentUser->name, IFace_currentUser->sip);
                }
                break;
            //Scroll to next contact
            case RIGHT:
                if (IFace_currentUser->next != NULL){
                    IFace_currentUser = IFace_currentUser->next;
                    LCD_writeMessage(IFace_currentUser->name, IFace_currentUser->sip);
                }
                break;
            //Start call
            case IN:
                if (IFace_currentUser->prev != NULL){
                    printf("Calling user '%s' at %s\n", IFace_currentUser->name, IFace_currentUser->sip);
                    pjsua_interface_make_call(IFace_currentUser->sip);
                    LCD_writeMessage("Calling...", IFace_currentUser->name);
                }
                break;
            default:
            break;
        }
    }

    return NULL;
}

void IFace_cleanup() {
    //remove thread, delete all users
    IFace_running = false;

    pthread_join(tid, NULL);

    struct IFace_user *next = IFace_lastUser->prev;
    for(IFace_currentUser = IFace_lastUser; IFace_currentUser != NULL; IFace_currentUser = next){
        printf("FREE: %s\n", IFace_currentUser->name);
        free(IFace_currentUser->name);
        free(IFace_currentUser->sip);
        next = IFace_currentUser->prev;
        free(IFace_currentUser);
    }
}