/* 16x2 LCD Initialization C Sample Code Modified by Group 404*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "gpio.h"

// nanosleep wrapper function - accepts seconds and nanoseconds to construct delay
static void delayFor(int, int);
// Flash the E pin high to low to have the LCD consume the data
static void pulseEnable();
// Write 4 bits to their corresponding pin (D4, D5, D6, D7)
static void write4Bits(uint8_t);
// Write a char to the LCD
static void writeChar(char);
// Write a message to the LCD to line 1 and 2
void writeMessage(char* msg1, char* msg2);
// Clear the LCD
void clearMessage();
// Initialize the LCD
void LCD_initialize();

/*int main()
{
    initializeLCD();
    writeMessage("Hello world!", "HIiiiiiiiiiiiiiiiIIIiiIii");
    //clearMessage();
    return 0;
}*/

void LCD_initialize()
{
    // Set every GPIO pin to OUTPUT
    GPIO_writeDirection(RS_GPIO_NUMBER, "out");
    GPIO_writeDirection(E_GPIO_NUMBER, "out");
    GPIO_writeDirection(D4_GPIO_NUMBER, "out");
    GPIO_writeDirection(D5_GPIO_NUMBER, "out");
    GPIO_writeDirection(D6_GPIO_NUMBER, "out");
    GPIO_writeDirection(D7_GPIO_NUMBER, "out");

    // Set every data pin to 0
    GPIO_writeValue(E_GPIO_NUMBER, "0");
    GPIO_writeValue(D4_GPIO_NUMBER, "0");
    GPIO_writeValue(D5_GPIO_NUMBER, "0");
    GPIO_writeValue(D6_GPIO_NUMBER, "0");
    GPIO_writeValue(D7_GPIO_NUMBER, "0");

    printf("Configured pins...\n");

    // Set to command mode
    GPIO_writeValue(RS_GPIO_NUMBER, "0");

    // Special Function Set 1.
	write4Bits(0x03); // 0011
    delayFor(0, 5000000); // 5 ms

	// Special Function Set 2.
	write4Bits(0x03); // 0011
    delayFor(0, 128000); // 128 us
    
	// Special Function Set 3.
	write4Bits(0x03); // 0011
    delayFor(0, 128000); // 128 us

	// Sets to 4-bit operation.
	write4Bits(0x2); /* 0010 */
	delayFor(0, 1000000); // 1 ms

	// Sets to 4-bit operation. Sets 2-line display. Selects 5x8 dot character font.
	write4Bits(0x2); /* 0010 - We can alternatively write 0000 here for 8-bit operation. */
	write4Bits(0x8); /* 1000 - We can alternatively write 1100 here for 5x10 dot font. */
    delayFor(0, 128000); // 128 us

	// Display ON/OFF control.
	write4Bits(0x0); /* 0000 */
	write4Bits(0x8); /* 1000 */
	delayFor(0, 128000); // 128 us

    // Clear the display.
	write4Bits(0x0); /* 0000 */
	write4Bits(0x1); /* 0001 */
	delayFor(0, 64000); // 64 us

	// Sets mode to increment cursor position by 1 and shift right when writing to display.
	write4Bits(0x0); /* 0000 */
	write4Bits(0x6); /* 0110 */
	delayFor(0, 128000); // 128 us

	// Turns on display. This corresponds to the instruction 0000 1100 in binary.
	// To be able to see the cursor, use 0000 1110.
	// To enable cursor blinking, use 0000 1111.
	write4Bits(0x0); /* 0000 */
	write4Bits(0xF); /* 1111 */
	delayFor(0, 64000); // 64 us

    printf("Completed initialization.\n");

	// Pull RS up to write data.
	GPIO_writeValue(RS_GPIO_NUMBER, "1");
}

void LCD_clearMessage()
{
    GPIO_writeValue(RS_GPIO_NUMBER, "0");

    // Clear the display.
	write4Bits(0x0); /* 0000 */
	write4Bits(0x1); /* 0001 */
	delayFor(0, 64000); // 64 us

    GPIO_writeValue(RS_GPIO_NUMBER, "1");

}

void LCD_writeMessage(char* msg1, char* msg2)
{   
    LCD_clearMessage();
    //printf("Writing \"%s\" to LCD...\n", msg1);
    for (int i = 0; i < strlen(msg1); i++) {
        writeChar(msg1[i]);
    }
    for (int i = 0; i < 40 - strlen(msg1); i++) {
        writeChar(' ');
    }
    for (int i = 0; i < strlen(msg2); i++) {
        writeChar(msg2[i]);
    }
}

static void writeChar(char c)
{
	unsigned int upper_bits = (c >> 4);
	unsigned int lower_bits = c & 0xF;
	write4Bits(upper_bits);
	write4Bits(lower_bits);
}

static void pulseEnable() {
    struct timespec pulseDelay = {0, 10000000};
    GPIO_writeValue(E_GPIO_NUMBER, "1");
    nanosleep(&pulseDelay, (struct timespec*) NULL);
    GPIO_writeValue(E_GPIO_NUMBER, "0");
    nanosleep(&pulseDelay, (struct timespec*) NULL);
}

static void write4Bits(uint8_t value)
{
    char strBit[2];
    strBit[1] = '\0'; 
    strBit[0] = (value & 0x01 ? 1 : 0) + '0';
	GPIO_writeValue(D4_GPIO_NUMBER, strBit);

    strBit[0] = ((value >> 1) & 0x01 ? 1 : 0) + '0';
    GPIO_writeValue(D5_GPIO_NUMBER, strBit);

    strBit[0] = ((value >> 2) & 0x01 ? 1 : 0) + '0';
    GPIO_writeValue(D6_GPIO_NUMBER, strBit);

    strBit[0] = ((value >> 3) & 0x01 ? 1 : 0) + '0';
    GPIO_writeValue(D7_GPIO_NUMBER, strBit);
    
	pulseEnable();
}

static void delayFor(int s, int ns)
{
    struct timespec delay = {s, ns};
    nanosleep(&delay, (struct timespec*) NULL);
}