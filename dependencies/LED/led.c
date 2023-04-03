//Implementation of LED headers
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "led.h"
#include "../utils/util.h"

#define GPIO_EXPORT_DIRECTORY "/sys/class/gpio/export"
#define GPIO_NUMBER_DIRECTORY GPIO_DIR "49/"
#define GPIO_NUMBER "49"
#define LED_ON "0"
#define LED_OFF "1"
#define TRUE (1 == 1)
#define FALSE (1 != 1)

static pthread_t blink_thread = -1;

//returns the ms required for delay_off/delay_on given hz.
static inline unsigned int hz_to_ms(const unsigned int hz){
    //divide by half the actual hz because need to figure out the setting for duration on *and* off.
    return 1000 / (hz * 2);
}

void LED_turnOn(){
    writeToFile(GPIO_NUMBER_DIRECTORY VALUE, LED_ON);
}

void LED_turnOff(){
    writeToFile(GPIO_NUMBER_DIRECTORY VALUE, LED_OFF);
}

//init to false
static int isBlinking = FALSE;

static void makeBlink(void * input){
    const unsigned int ms = *(unsigned int *)input;
    printf("ms = %d\n", *(unsigned int *)input);
    free(input);
    while(isBlinking){
        LED_turnOn();
        sleepMs(ms);
        LED_turnOff();
        sleepMs(ms);
    }
}

//blink an LED with a specific frequency.
void LED_blink(const unsigned int hz){
    unsigned int * ms = malloc(sizeof(unsigned int));
    *ms = hz_to_ms(hz);
    // printf("ms = %d\n", ms);
    isBlinking = TRUE;
    pthread_create(&blink_thread, NULL, (void *)&makeBlink, (void *)ms);
}

//stops the LED blink.
void LED_stopBlink(){
    isBlinking = FALSE;
    pthread_join(blink_thread, NULL);
    LED_turnOff();
}

//set up the GPIO, including exporting it if it isn't already created.
static void setGPIO(void){
    if(!isGPIOPinExported(GPIO_NUMBER_DIRECTORY)){
        writeToFile(GPIO_EXPORT_DIRECTORY, GPIO_NUMBER);
        sleepMs(300);
    }
    writeToFile(GPIO_NUMBER_DIRECTORY DIRECTION, "out");
    //have LED off by default.
    LED_turnOff();
}

void LED_startUp(void){
    setGPIO();
}

void LED_cleanUp(void){
    if(blink_thread != -1){
        isBlinking = FALSE;
        pthread_join(blink_thread, NULL);
    }
    LED_turnOff();
}