#include "../utils/util.h"
#include "buzzer.h"
#include <stdio.h>
#include <pthread.h>



int main(void){
    buzzer_init();
    buzzer_ring_on();
    sleepMs(7000);
    printf("Turning this off!\n");
    buzzer_ring_off();
    buzzer_cleanup();
    return 0;
}