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
#ifndef LEDrtio
#define LEDrtio

typedef enum {
    LED_ZERO = 0,
    LED_ONE,
    LED_TWO,
    LED_THREE,
    LED_COUNT
}LED_number;

//Turn on a specific LED.
void LED_turnOn(LED_number LED);

//Turn off a specific LED.
void LED_turnOff(LED_number LED);

//Turn on all LEDs (and do not turn them off)
void LED_turnOnAll(void);
//Turn off all LEDs.
void LED_turnOffAll(void);
//turns on an LED for a specified number of microseconds
void LED_turnOnms(LED_number LED, const unsigned long long microseconds);
//Turns on all LEDs for a specified number of microseconds
void LED_turnOnAllms(const unsigned long long microseconds);

//blink an led for a specific frequency for specified ms. Will sleep / block execution while blinking.
void LED_blink(LED_number LED, const unsigned int hz, const unsigned long long duration_ms);
//blink all LEDs with hz frequency. Will sleep / block program execution while blinking.
void led_blinkAll(const unsigned int hz, const unsigned long long duration_ms);

//blink an LED with a specific frequency. Does not block, but requires 300ms of time before blinking begins.
void LED_blink_noblock(LED_number LED, const unsigned int hz);
//stops an LED blink.
void LED_stopBlink(LED_number LED);

//startup function. Call before anything else.
void LED_startUp(void);
//cleanup function. Use to reset LEDs to default.
void LED_cleanUp(void);
#endif