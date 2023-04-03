#Makefile sample
OUTFILE = potTest
OUTDIR = $(HOME)/cmpt433/public/

#Change cfile here for different compilation
CFILE =  $(HOME)/433_utils/util/utils/util.c pot.c potTest.c

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -pthread

build:
	$(CC_C) $(CFLAGS) $(CFILE) -o $(OUTFILE)
	cp $(OUTFILE) $(OUTDIR)
