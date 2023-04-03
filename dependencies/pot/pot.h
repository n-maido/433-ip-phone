/*
    Code to get the reading for the potentiometer on the beaglebone 
        -By Ryan Tio
    Usage: Call readValue to get the direct reading of the pin (non-voltage).
           It's really that simple.
*/

#ifndef rtiopot
#define rtiopot


void POT_init(void);
void POT_cleanUp(void);

//return the direct reading (non-voltage) of the potentiometer.
unsigned int POT_readValue(void);

#endif