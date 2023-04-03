#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "led.h"
#include "../utils/util.h"


int main(){
    //test change trigger
    // changeTrigger(LED0_TRIGGER, "heartbeat");
    // changeTrigger(LED1_TRIGGER, "heartbeat");
    // changeTrigger(LED2_TRIGGER, "heartbeat");
    // changeTrigger(LED3_TRIGGER, "heartbeat");
    // setLEDsDefault();
    // setAllLEDsTimer();
    // changeTrigger
    // setLEDTimer(LED2_TRIGGER);
    // LED_blink(LED2_PATH DELAY_OFF, LED2_PATH DELAY_ON, 10);
    // LED_turnOnms(LED1_BRIGHTNESS, 3500);
    // for(int i = 0; i < 1000; i++){
    //     setLEDsDefault();
        // sleepMs(100);
        // setAllLEDsTimer();
    LED_startUp();
    // setAllLEDSNone();
    // led_blinkAll(3, 1000);
    LED_blink(8);
    sleepMs(5000);
    LED_stopBlink();
    sleepMs(5000);
    LED_cleanUp();
    // }
    
    return 0;
}