PJDIR = /home/san/pjsua/pjproject-2.13
include $(PJDIR)/build.mak
OUTDIR = $(HOME)/cmpt433/public/projectApps
OUTFILE = $(OUTDIR)/thread_pjsua_module
#OUTFILE = thread_pjsua_module_binary
CC=arm-linux-gnueabihf-gcc
CFLAGS=-Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow


$(OUTFILE): main.c pjsua_interface.c pjsua_interface.h
	$(PJ_CC) -o $(OUTFILE)  $< pjsua_interface.c $(PJ_CFLAGS) $(PJ_LDFLAGS) $(PJ_LDLIBS)


all: $(OUTFILE)
clean:
	rm -f $(OUTFILE) $