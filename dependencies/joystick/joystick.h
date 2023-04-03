// joystick.h
// Module to read what direction the joystick is pressed in
#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

//Possible directions the joystick may be pushed in
enum JS_direction {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    IN
};

//Initialize the joystick to be read through gpio
void JS_initialize();
//Returns the joystick's current direction
enum JS_direction JS_read();

#endif