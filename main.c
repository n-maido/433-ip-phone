/**
 * Entrypoint for the IP Phone app
 * By Ryan Tio & Nhi Mai-Do, modified from assignment 3 code
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "udp_server/udp_server.h"

static pthread_cond_t shutdownCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t conditionMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t shutdownPid = -1;

void shutdown(void);

//prepare all of the modules.
void init(void){
    //allow any of the modules to shutdown the program if it breaks.
    udp_init(&shutdownCondition, &conditionMutex);
}

//shutdown all of the modules.
void shutdown(void){
    udp_cleanup();
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
    shutdown();

    return 0;
}