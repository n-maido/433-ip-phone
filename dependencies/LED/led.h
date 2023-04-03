/*
    Beaglebone Segment display header by Ryan Tio
    Usage / cautions:

        startUp() must be called before anything else.
        cleanUp() may be called at the end of execution, but will not have any ill side effects (memory leaks, open threads, etc) if it is omitted.
            -cleanUp() essentially sets the LED behaviour back to default, so it isn't that much of an issue if it is omitted.

        Provides support for:
            -turning on LEDs (for a specified time or otherwise)
            -turning off LEDs
            -Blinking LED(s) at a specific frequency for a given time.
        Note that the LED_number enum must be used where necessary.
*/
#ifndef LEDrtio_gpio
#define LEDrtio_gpio

//Turn on the LED.
void LED_turnOn();

//Turn off the LED.
void LED_turnOff();

//blink an LED with a specific frequency. Does NOT block. call stopBlink() before starting this again.
void LED_blink(const unsigned int hz);
//stops an LED blink.
void LED_stopBlink();

//startup the LED
void LED_startUp(void);
//cleanup the LED
void LED_cleanUp(void);
#endif