//Implementation of LED headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "led.h"
#include "../utils/util.h"

#define LED_PATH "/sys/class/leds/beaglebone:green:usr"

#define LED0_PATH LED_PATH "0/"
#define LED0_TRIGGER LED0_PATH "trigger"
#define LED0_BRIGHTNESS LED0_PATH "brightness"

#define LED1_PATH LED_PATH "1/"
#define LED1_TRIGGER LED1_PATH "trigger"
#define LED1_BRIGHTNESS LED1_PATH "brightness"

#define LED2_PATH LED_PATH "2/"
#define LED2_TRIGGER LED2_PATH "trigger"
#define LED2_BRIGHTNESS LED2_PATH "brightness"

#define LED3_PATH LED_PATH "3/"
#define LED3_TRIGGER LED3_PATH "trigger"
#define LED3_BRIGHTNESS LED3_PATH "brightness"

#define DELAY_OFF "delay_off"
#define DELAY_ON "delay_on"

//returns the ms required for delay_off/delay_on given hz.
static inline unsigned int hz_to_ms(const unsigned int hz){
    //divide by half the actual hz because need to figure out the setting for duration on *and* off.
    return 1000 / (hz * 2);
}

static void changeTrigger(const char * led_trigger, const char * option){
    writeToFile(led_trigger, option);
}

#define LED0_DEFAULT "heartbeat"
#define LED1_DEFAULT "mmc0"
#define LED2_DEFAULT "cpu0"
#define LED3_DEFAULT "mmc1"
static void setLEDsDefault(void){
    changeTrigger(LED0_TRIGGER, LED0_DEFAULT);
    changeTrigger(LED1_TRIGGER, LED1_DEFAULT);
    changeTrigger(LED2_TRIGGER, LED2_DEFAULT);
    changeTrigger(LED3_TRIGGER, LED3_DEFAULT);
}

static void changeAllLEDTrigger(const char * option){
    changeTrigger(LED0_TRIGGER, option);
    changeTrigger(LED1_TRIGGER, option);
    changeTrigger(LED2_TRIGGER, option);
    changeTrigger(LED3_TRIGGER, option);
}

static void setAllLEDSNone(void){
    changeAllLEDTrigger("none");
}

// static void setLEDNone(const char * led_trigger_path){
//     changeTrigger(led_trigger_path, "none");
// }

void LED_turnOn(LED_number LED){
    char toWrite[2] = "1";
    switch(LED){
        case LED_ZERO:
            writeToFile(LED0_BRIGHTNESS, toWrite);
            break;
        case LED_ONE:
            writeToFile(LED1_BRIGHTNESS, toWrite);
            break;
        case LED_TWO:
            writeToFile(LED2_BRIGHTNESS, toWrite);
            break;
        case LED_THREE:
            writeToFile(LED3_BRIGHTNESS, toWrite);
            break;
        case LED_COUNT:
            break;
    }
}

void LED_turnOff(LED_number LED){
        char toWrite[2] = "0";
    switch(LED){
        case LED_ZERO:
            writeToFile(LED0_BRIGHTNESS, toWrite);
            break;
        case LED_ONE:
            writeToFile(LED1_BRIGHTNESS, toWrite);
            break;
        case LED_TWO:
            writeToFile(LED2_BRIGHTNESS, toWrite);
            break;
        case LED_THREE:
            writeToFile(LED3_BRIGHTNESS, toWrite);
            break;
        case LED_COUNT:
            break;
    }
}

void LED_turnOnAll(void){
    writeToFile(LED0_BRIGHTNESS, "1");
    writeToFile(LED1_BRIGHTNESS, "1");
    writeToFile(LED2_BRIGHTNESS, "1");
    writeToFile(LED3_BRIGHTNESS, "1");
}

void LED_turnOffAll(void){
    writeToFile(LED0_BRIGHTNESS, "0");
    writeToFile(LED1_BRIGHTNESS, "0");
    writeToFile(LED2_BRIGHTNESS, "0");
    writeToFile(LED3_BRIGHTNESS, "0");
}

//blink an LED with a specific frequency.
void LED_blink_noblock(LED_number LED, const unsigned int hz){
    unsigned int ms = hz_to_ms(hz);
    char numBuff[20] = "";
    snprintf(numBuff, 20, "%d", ms);
    switch (LED)
    {
    case LED_ZERO:
        writeToFile(LED0_TRIGGER, "timer");
        sleepMs(300);
        writeToFile(LED0_PATH DELAY_ON, numBuff);
        writeToFile(LED0_PATH DELAY_OFF, numBuff);
        break;
    case LED_ONE:
        writeToFile(LED1_TRIGGER, "timer");
        sleepMs(300);
        writeToFile(LED1_PATH DELAY_ON, numBuff);
        writeToFile(LED1_PATH DELAY_OFF, numBuff);
        break;
    case LED_TWO:
        writeToFile(LED2_TRIGGER, "timer");
        sleepMs(300);
        writeToFile(LED2_PATH DELAY_ON, numBuff);
        writeToFile(LED2_PATH DELAY_OFF, numBuff);
        break;
    case LED_THREE:
        writeToFile(LED3_TRIGGER, "timer");
        sleepMs(300);
        writeToFile(LED3_PATH DELAY_ON, numBuff);
        writeToFile(LED3_PATH DELAY_OFF, numBuff);
        break;
    default:
        break;
    }
}

//stops an LED blink.
void LED_stopBlink(LED_number LED){
    const char trig[6] = "none";
    switch (LED)
    {
    case LED_ZERO:
        writeToFile(LED0_TRIGGER, trig);
        break;
    case LED_ONE:
        writeToFile(LED1_TRIGGER, trig);
        break;
    case LED_TWO:
        writeToFile(LED2_TRIGGER, trig);
        break;
    case LED_THREE:
        writeToFile(LED3_TRIGGER, trig);
        break;
    default:
        break;
    }
}

void LED_turnOnms(LED_number LED, const unsigned long long microseconds){
    LED_turnOn(LED);
    sleepMs(microseconds);
    LED_turnOff(LED);
}

void LED_turnOnAllms(const unsigned long long microseconds){
    LED_turnOnAll();
    sleepMs(microseconds);
    LED_turnOffAll();
}

void LED_blink(LED_number LED, const unsigned int hz, const unsigned long long duration_ms){
    unsigned int ms = hz_to_ms(hz);
    unsigned int flashes = duration_ms / (2 * ms);
    for(unsigned int i = 1; i <= flashes; ++i){
        LED_turnOnms(LED, ms);
        LED_turnOff(LED);
        sleepMs(ms);
    }
}

void led_blinkAll(const unsigned int hz, const unsigned long long duration_ms){
    unsigned int ms = hz_to_ms(hz);
    unsigned int flashes = duration_ms / (2 * ms);
    for(unsigned int i = 1; i <= flashes; ++i){
        LED_turnOnAllms(ms);
        LED_turnOffAll();
        sleepMs(ms);
    }
}

void LED_startUp(void){
    setAllLEDSNone();
}

void LED_cleanUp(void){
    setLEDsDefault();
}