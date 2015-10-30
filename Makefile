
SRC = cue2pops.c

CC = gcc
CFLAGS = -Wall -Wextra

TARGET = cue2pops

INSTALL_DIR = /usr/local/bin

RM = rm

CP = cp

cue2pops:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

all: cue2pops

install:
	$(CP) $(TARGET) $(INSTALL_DIR)

clean:
	$(RM) $(TARGET)

