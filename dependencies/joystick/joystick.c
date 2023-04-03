#include "../utils/util.h"
#include <stdio.h>
#include <stdlib.h>

#define GPIO_PATH "/sys/class/gpio/gpioXX/value"
#define GPIO_CHAR_INDEX 20
#define CONFIG_JOYSTICK_UP_GPIO "config-pin p8.14 gpio\necho in > /sys/class/gpio/gpio26/direction"
#define CONFIG_JOYSTICK_DOWN_GPIO "config-pin p8.16 gpio\necho in > /sys/class/gpio/gpio46/direction"
#define CONFIG_JOYSTICK_LEFT_GPIO "config-pin p8.18 gpio\necho in > /sys/class/gpio/gpio65/direction"
#define CONFIG_JOYSTICK_RIGHT_GPIO "config-pin p8.15 gpio\necho in > /sys/class/gpio/gpio47/direction"
#define CONFIG_JOYSTICK_IN_GPIO "config-pin p8.17 gpio\necho in > /sys/class/gpio/gpio27/direction"
#define GPIO_JOYSTICK_UP_PIN "26"
#define GPIO_JOYSTICK_DOWN_PIN "46"
#define GPIO_JOYSTICK_LEFT_PIN "65"
#define GPIO_JOYSTICK_RIGHT_PIN "47"
#define GPIO_JOYSTICK_IN_PIN "27"

enum JS_direction {
    NONE,
    DOWN,
    UP,
    RIGHT,
    LEFT,
    IN
};

void JS_initialize() {
    runCommand(CONFIG_JOYSTICK_UP_GPIO);
    runCommand(CONFIG_JOYSTICK_DOWN_GPIO);
    runCommand(CONFIG_JOYSTICK_LEFT_GPIO);
    runCommand(CONFIG_JOYSTICK_RIGHT_GPIO);
    runCommand(CONFIG_JOYSTICK_IN_GPIO);
}

static int JS_readPin(char* pin){
    char fullPath[] = GPIO_PATH;
    fullPath[GPIO_CHAR_INDEX] = pin[0];
    fullPath[GPIO_CHAR_INDEX + 1] = pin[1];

    FILE *pFile = fopen(fullPath, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", fullPath);
        exit(-1);
    }
    // Read string (line)
    char buff[2];
    fgets(buff, 2, pFile);

    // Close
    fclose(pFile);

    return (int)buff[0] - '0';
}

enum JS_direction JS_read() {
    if (!JS_readPin(GPIO_JOYSTICK_UP_PIN)){
        return UP;
    }
    if (!JS_readPin(GPIO_JOYSTICK_DOWN_PIN)){
        return DOWN;
    }
    if (!JS_readPin(GPIO_JOYSTICK_LEFT_PIN)){
        return LEFT;
    }
    if (!JS_readPin(GPIO_JOYSTICK_RIGHT_PIN)){
        return RIGHT;
    }
    if (!JS_readPin(GPIO_JOYSTICK_IN_PIN)){
        return IN;
    }
    return NONE;
}