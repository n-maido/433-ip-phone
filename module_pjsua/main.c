/**
 * Sanseerat virk
 * starting point to run just the pjsua module
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "pjsua_interface.h"

static pthread_cond_t shutdownCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t conditionMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t shutdownPid = -1;

void shutdown_app(void);

//prepare all of the modules.
void init(void){
    //allow any of the modules to shutdown the program if it breaks.
    pjsua_interface_init(&shutdownCondition, &conditionMutex);
    //display inint 
}

//shutdown all of the modules.
void shutdown_app(void){
    pjsua_interface_cleanup();
    //display clean up
}

//awaits a conditional variable, halting execution of the main thread until another thread signals.
void waitForShutdown(void){
    pthread_mutex_lock(&conditionMutex);
    pthread_cond_wait(&shutdownCondition, &conditionMutex);
    pthread_mutex_unlock(&conditionMutex);
    return;
}

int main(){


    if(pthread_create(&shutdownPid, NULL, (void *)&waitForShutdown, NULL) != 0){
        perror("Error creating shutdown thread.");
        return -1;
    }

    init();

    pthread_join(shutdownPid, NULL);
    printf("Received shutdown request.\n");
    shutdown_app();

    return 0;
}