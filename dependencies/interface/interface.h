// interface.h
// Module to allow the user to see contacts and use joystick to call or scroll
#ifndef _INTERFACE_H_
#define _INTERFACE_H_

//Create thread, create default user and initialize current / last user, initialize joystick and lcd
void IFace_initialize();

//Add a user to the interface
void IFace_addUser(char* username, char* sip);

//Remove a user from the interface
void IFace_removeUser(char* sip);

//Update the display in case of incoming call or such
void IFace_updateStatus(int status, char* address);

//Main thread
void* IFace_runner(void* arg);

//Ends the thread
void IFace_endThread();

//remove thread, delete all users
void IFace_cleanup();

#endif