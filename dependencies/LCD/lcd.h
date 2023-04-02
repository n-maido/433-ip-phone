#ifndef _LCD_H_
#define _LCD_H_

// Write a message to the LCD to line 1 and 2
void writeMessage(char* msg1, char* msg2);
// Clear the LCD
void clearMessage();
// Initialize the LCD
void LCD_initialize();

#endif