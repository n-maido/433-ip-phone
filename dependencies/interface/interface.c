#include "../joystick/joystick.h"
#include "../LCD/lcd.h"
#include "../utils/util.h"
#include "interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

//Linked list of users
struct IFace_user {
    char* name;
    char* sip;
    struct IFace_user *prev;
    struct IFace_user *next;
};

static pthread_t tid;
static pthread_attr_t attr;

static struct IFace_user *IFace_currentUser;
static struct IFace_user *IFace_lastUser;
static bool IFace_running = false;

static struct IFace_user* IFace_createUser(char* username, char* sip);
static void* IFace_runner(void* arg);

/*int main(){
    IFace_initialize();
    IFace_addUser("test1", "1");
    IFace_addUser("test2", "2");
    IFace_addUser("test3", "3");
    sleepMs(30000);
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

    char* user_name = (char*)malloc(sizeof(char) * strlen(username));
    char* user_sip = (char*)malloc(sizeof(char) * strlen(sip));

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

static void* IFace_runner(void* arg) {
    //main interface loop
    while(IFace_running){
        enum JS_direction input = JS_read();
        while (input == NONE) {
            input = JS_read();
        }
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
            //Start or End call
            case IN:
                //TODO: CALL CALLING FUNCTION
                printf("Calling/Ending call with user '%s' at %s\n", IFace_currentUser->name, IFace_currentUser->sip);
                LCD_writeMessage("Calling...", IFace_currentUser->name);
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
    
    struct IFace_user *next = IFace_currentUser->prev;
    for(IFace_currentUser = IFace_lastUser; IFace_currentUser != NULL; IFace_currentUser = next){
        free(IFace_currentUser->name);
        free(IFace_currentUser->sip);
        next = IFace_currentUser->prev;
        free(IFace_currentUser);
    }

    pthread_join(tid, NULL);
}