/*
    UDP header by Ryan Tio and Nhi Mai-Do, modified form assignment 3
    Usage / cautions:
        -pthread must be added to the compiler flags. 

        startUp() must be called before anything else.
        cleanUp() must be called at the end of execution in order to clear the 8-segment display. 
            If cleanUp is not successfully called:
            -There may be unclosed file descriptors
            -There may be an unclosed thread. 
*/

#ifndef udp_server
#define udp_server

#include <pthread.h>

//Set up UDP socket and thread. Returns -1 upon failure.
int udp_init(pthread_cond_t * cond, pthread_mutex_t * lock);
int send_info_message(void);

// Notifies the udp module that a call is in progress
// Forwards the notification to the web interface
void udp_notifyCallStarted(char* calleeAddress);

//Cleanup. Returns -1 if there were any issues closing.
int udp_cleanup(void);

#endif