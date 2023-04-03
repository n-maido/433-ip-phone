# Makefile for IP phone app
# Locally compile on bbg
# Based on the sample A3 Makefile by Brian Fraser

#pjsua dependencies 

PJDIR = /mnt/remote/pjsip-apps/pjproject-2.13
include $(PJDIR)/build.mak

# Edit this file to compile extra C files into their own programs.
TARGET= ip_phone
SOURCES= main.c udp_server/udp_server.c utils/utils.c module_pjsua/pjsua_interface.c module_pjsua/pjsua_interface.h dependencies/utils/util.c dependencies/buzzer/buzzer.c dependencies/interface/interface.c dependencies/LCD/lcd.c dependencies/LCD/gpio.c dependencies/joystick/joystick.c dependencies/pot/pot.c dependencies/LED/led.c dependencies/ipaddr/ipaddr.c

# PUBDIR = $(HOME)/cmpt433/public/myApps
OUTDIR = /mnt/remote/
# CROSS_TOOL = arm-linux-gnueabihf-
# CC_CPP = $(CROSS_TOOL)g++
# CC_C = $(CROSS_TOOL)gcc

#CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow

# Asound Library
# - See the AudioGuide for steps to copy library from target to host.
LFLAGS = -L/mnt/remote/asound_lib_BBB

# -pg for supporting gprof profiling.
#CFLAGS += -pg


all: node
	$(PJ_CC) $(SOURCES) -o $(OUTDIR)/$(TARGET) $(PJ_CFLAGS) $(PJ_LDFLAGS) $(PJ_LDLIBS)

build:
	$(PJ_CC) $(SOURCES) -o $(OUTDIR)/$(TARGET) $(PJ_CFLAGS) $(PJ_LDFLAGS) $(PJ_LDLIBS)

clean:
	rm -f $(OUTDIR)/$(TARGET)

# Copy the nodeJS server to the public directory.
node:
	mkdir -p $(OUTDIR)ip_phone_node_copy/ 
	cp -R node_server/* $(OUTDIR)ip_phone_node_copy/ 
#cd $(PUBDIR)/ip_phone_node_copy/ && npm install