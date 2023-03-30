#include "buzzer.h"
#include <stdio.h>
#include <stdlib.h>
#include "../utils/util.h"
#include "pthread.h"

#define INIT_GPIO_CMD "config-pin p9_22 pwm"
#define DIR_PREFIX "/dev/bone/pwm/0/a/"
#define ENABLE_DIR DIR_PREFIX "enable"
#define DUTY_CYCLE_DIR DIR_PREFIX "duty_cycle"
#define PERIOD_DIR DIR_PREFIX "period"

void buzzer_init(void){
    runCommand(INIT_GPIO_CMD);
    sleepMs(300);
    //test code here
    writeToFile(PERIOD_DIR, "1500000");
    writeToFile(DUTY_CYCLE_DIR, "750000");
    // writeToFile(ENABLE_DIR, "1");
}

//stop the godforsaken buzzing
void buzzer_cleanup(void){
    writeToFile(ENABLE_DIR, "0");
}

//turns the buzzer on. 
static void buzzer_on(void){
    writeToFile(ENABLE_DIR, "1");
}


//turns the buzzer off.
static void buzzer_off(void){
    writeToFile(ENABLE_DIR, "0");
}

static int isRing = 1;
static pthread_t buzzer_ring_pid = -1;

static void make_ring(void){
    long long count = 1;
    while(isRing != 0){
        buzzer_on();
        sleepMs(110);
        buzzer_off();
        sleepMs(5);
        if(count % 11 == 0){
            sleepMs(1000);
        }
        count++;
        
    }
}

void buzzer_ring_on(void){
    isRing = 1;
    pthread_create(&buzzer_ring_pid, NULL, (void*)&make_ring, NULL);
}
void buzzer_ring_off(void){
    isRing = 0;
    pthread_join(buzzer_ring_pid, NULL);
}