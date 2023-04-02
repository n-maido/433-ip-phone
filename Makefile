# Makefile for IP phone app
# Based on the sample A3 Makefile by Brian Fraser

# Edit this file to compile extra C files into their own programs.
TARGET= ip_phone
SOURCES= main.c udp_server/udp_server.c utils/utils.c

PUBDIR = $(HOME)/cmpt433/public/myApps
OUTDIR = $(PUBDIR)
CROSS_TOOL = arm-linux-gnueabihf-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow

# Asound Library
# - See the AudioGuide for steps to copy library from target to host.
LFLAGS = -L$(HOME)/cmpt433/public/asound_lib_BBB

# -pg for supporting gprof profiling.
#CFLAGS += -pg


all: node
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(OUTDIR)/$(TARGET)  $(LFLAGS) -lpthread -lasound

clean:
	rm -f $(OUTDIR)/$(TARGET)

# Copy the nodeJS server to the public directory.
node:
	mkdir -p $(PUBDIR)/ip_phone_node_copy/ 
	cp -R node_server/* $(PUBDIR)/ip_phone_node_copy/ 
# cd $(PUBDIR)/ip_phone_node_copy/ && npm install