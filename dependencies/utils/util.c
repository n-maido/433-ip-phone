//Implementation for util.h.

#include "util.h"
#include "conversion.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <dirent.h> //for makedir
#include <stdbool.h>

unsigned int readA2DVoltage(const char * pin){
    //open the provided pin file
    FILE *pFile = fopen(pin, "r");
    //exit if opening fails and print a helpful error
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", pin);
        perror("");
        return -1;
    }
    // Read string (line)
    const int MAX_LENGTH = 6;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);
    // Close
    fclose(pFile);
    return atoi(buff);
    // return 0;
}

void writeToFile(const char * path, const char * str){
    FILE * f = fopen(path, "w");
    if(f == NULL){
        printf("Opening %s failed.\n", path);
        exit(1);
    }
    int charWritten = fprintf(f, str);
    if(charWritten <= 0 ){
        printf("Writing to %s failed", path);
        exit(1);
    }
    fclose(f);
}

//taken directly from Dr. Fraser's notes.
long long currentTimeInMs(void){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000
    + nanoSeconds / 1000000;
    return milliSeconds;
}

//taken (almost) directly from Dr. Fraser's notes.
void sleepMs(const unsigned long long delayInMs){
    // long long ns = MS_TO_NS(delayInMs);
    int seconds = MS_TO_SEC(delayInMs);
    int nanoseconds = MS_TO_NS(delayInMs) % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

//taken directly from Dr. Fraser's notes.
void runCommand(const char* command){
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");
    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
    if (fgets(buffer, sizeof(buffer), pipe) == NULL)
    break;
    // printf("--> %s", buffer); // Uncomment for debugging
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
    perror("Unable to execute command:");
    printf(" command: %s\n", command);
    printf(" exit code: %d\n", exitCode);
    }
}

void setPinDirection(const char* pin_direction_path, const char* direction){
    writeToFile(pin_direction_path, direction);
}

//reads a pin and returns an integer representing the low/hi.
unsigned int readVoltage(const char * pin){
    //open the provided pin file
    FILE *pFile = fopen(pin, "r");
    //exit if opening fails and print a helpful error
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", pin);
        exit(-1);
    }
    // Read string (line)
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);
    // Close
    fclose(pFile);
    if(buff[0] == '0'){
        return 0;
    }else{
        return 1;
    }
}

void setPinActiveLow(const char * pin, const unsigned int active_low){
    if(active_low == 0){
        writeToFile(pin, "0");
    }else{
        writeToFile(pin, "1");
    }
    
}

bool isGPIOPinExported(const char * directory){
    DIR* dir = opendir(directory);
    if(dir){
        //directory exists (and therefore has already been exported.)
        closedir(dir);
        return true;
    }else if(ENOENT == errno){
        closedir(dir);
        //Directory doesn't exist.
        return false;
    }else{
        closedir(dir);
        //something else went wrong. Just let the program run and hope it sorts itself out.
        return true;
    }
}