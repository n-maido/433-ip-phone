OUTFILE = interface_test
OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow

app:
	$(CC_C) $(CFLAGS) interface.c ../joystick/joystick.c ../LCD/lcd.c ../LCD/gpio.c ../utils/util.c -pthread -o $(OUTDIR)/$(OUTFILE)
clean:
	rm $(OUTDIR)/$(OUTFILE)