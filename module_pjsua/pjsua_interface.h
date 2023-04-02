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

//pass 1 to pick up call and pass 2 to decline call
int pjsua_interface_pickup_incoming_call(int ack);


//hang up active call fuction make it safe if no call in session return false
int pjsua_interface_hang_up_call(void);


//0=no call in session , 1 = in comming call ringing, 2= call in session 3=ongoing call
int pjsua_interface_get_status_call(void);


//if call in session or inocoming uri of incoming call


//link wiht potionmeter


//add mic ampilier fucntions



//add output volume control fucntion 0 to 100
void pjsua_interface_set_volume(int volume)

int pjsua_interface_get_volume()

//automatic acount based on ip found on the interface


//Cleanup. Returns -1 if there were any issues closing.
int pjsua_interface_cleanup(void);

#endif