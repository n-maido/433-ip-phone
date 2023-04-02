/*
    Beaglebone utility header by Ryan Tio
    Usage / cautions:
        GPIO_DIR is intended to be used for accessing GPIO pins. Will cause issues if treated as the root directory
            -this is not the correct way of going about it, and will be changed in the future (a.k.a. after assignment 2)
        Otherwise, this is mainly used for writing to files, running shell commands, and sleep.
*/
#ifndef rtioutil
#define rtioutil

#include <stdbool.h>
//GPIO Macros
#define CONFIG_PIN "config-pin"
#define GPIO "gpio"
#define GPIO_DIR "/sys/class/gpio/gpio"
#define IN_DIRECTION "in"
#define OUT_DIRECTION "out"
#define DIRECTION "direction"
#define ACTIVE_LOW "active_low"
#define VALUE "value"


//Exits in the following cases: File failed to open, writing to file failed.
void writeToFile(const char * path, const char * str);
//Returns the current time since UNIX timestamp in milliseconds
long long currentTimeInMs(void);
//Runs the nanosleep() function for the provided milliseconds.
void sleepMs(const unsigned long long delayInMs);
//Runs the provided command in the linux shell.
void runCommand(const char* command);
// void runCommandWithResult(const char * command, char * return);
//set a GPIO pin's 'in'/'out' direction. Assumes that the pin has been exported beforehand.
void setPinDirection(const char* pin_direction_path, const char* direction);
//reads a pin and returns an integer representing the low/hi.
unsigned int readVoltage(const char * pin);
//Sets a pin's active low to the provided int. Expects 0/1.
void setPinActiveLow(const char * pin, const unsigned int active_low);
//Reads an A2D voltage and returns the reading unconverted to voltage.
unsigned int readA2DVoltage(const char * pin);
//credit to https://stackoverflow.com/questions/12510874/how-can-i-check-if-a-directory-exists
//for understanding on how to check if a directory exists.
bool isGPIOPinExported(const char * directory);

#endif
