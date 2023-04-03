#include "pot.h"
#include "../utils/util.h"

#define POT_pin "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"

void POT_init(void){
    //nothing to be done. Pin is already exported and set up by default.
    return;
}

void POT_cleanUp(void){
    //nothing to be done. 
    return;
}

unsigned int POT_readValue(void){
    return (unsigned int)(((double)readA2DVoltage(POT_pin) / 4095) * 100);
}