
SRC = cue2pops.c

CC = gcc
CFLAGS = -Wall -Wextra

CFLAGS_DEBUG = $(CFLAGS) -O0 -g -DDEBUG

TARGET = cue2pops

INSTALL_DIR = /usr/local/bin

RM = rm

CP = cp

all: cue2pops

cue2pops:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

debug:
	$(CC) $(CFLAGS_DEBUG) $(SRC) -o $(TARGET)


install: cue2pops
	$(CP) $(TARGET) $(INSTALL_DIR)

clean:
	$(RM) $(TARGET)

