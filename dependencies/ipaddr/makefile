#Makefile sample
OUTFILE = ipaddr
OUTDIR = $(HOME)/cmpt433/public/

#Change cfile here for different compilation
CFILE = main.c ipaddr.c

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -pthread

build:
	$(CC_C) $(CFLAGS) $(CFILE) -o $(OUTFILE)
	cp $(OUTFILE) $(OUTDIR)
