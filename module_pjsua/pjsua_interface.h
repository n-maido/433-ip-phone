/*
    Pjsua interface for application
    Sanseerat Virk
    Usage / cautions:
        -pthread must be added to the compiler flags. 

        startUp() must be called before anything else.
        cleanUp() must be called at the end of execution in order to clear the 8-segment display. 
            If cleanUp is not successfully called:
            -There may be unclosed file descriptors
            -There may be an unclosed thread. 
*/

#ifndef PJSUA_INTERFACE
#define PJSUA_INTERFACE

#include <pthread.h>

//Set up pjsua and thread. Returns -1 upon failure.
int pjsua_interface_init(pthread_cond_t * cond, pthread_mutex_t * lock);


//make call fucntion given a formatted sip uri 
int pjsua_interface_make_call(char *str);
// return succes or failure 


//hang up active call fuction make it safe if no call in session return false


//add mic ampilier fucntions

//add output volume control fucntion  


//Cleanup. Returns -1 if there were any issues closing.
int pjsua_interface_cleanup(void);

#endif