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
#define TRUE (1 == 1)
#define FALSE (1 == -1)

//returns the ms required for delay_off/delay_on given hz.
static inline unsigned int hz_to_ms(const unsigned int hz){
    //divide by half the actual hz because need to figure out the setting for duration on *and* off.
    return 1000 / (hz * 2);
}

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

static int isRing = FALSE;
static pthread_t buzzer_ring_pid = -1;

static void make_ring(void){
    long long count = 1;
    while(isRing){
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

static void make_ring_hz(void * input){
	double hz = *(double *)input;
	free(input);
	const int ms = (float)1000 / (hz * 2);
	long long count = 1;
	while(isRing){
		buzzer_on();
		sleepMs(ms);
		buzzer_off();
		sleepMs(ms);
		count++;
		if(count % 3 == 0){
			count = 1;
			sleepMs(1000);
		}
	}
}

void buzzer_ring(const double hz){
	if(hz < 0 || isRing){
		printf("Ring failed. Already ringing or hz out of range..");
	}else{
		isRing = TRUE;
		double * hz_in = malloc(sizeof(double));
		*hz_in = hz;
		pthread_create(&buzzer_ring_pid, NULL, (void*)&make_ring_hz, (void *)hz_in);
	}
}

void buzzer_ring_on(void){
	if(isRing){
		printf("Already ringing. Ring failed.");
	}else{
		isRing = TRUE;
    	pthread_create(&buzzer_ring_pid, NULL, (void*)&make_ring, NULL);
	}
    
}
void buzzer_ring_off(void){
    isRing = FALSE;
    pthread_join(buzzer_ring_pid, NULL);
}