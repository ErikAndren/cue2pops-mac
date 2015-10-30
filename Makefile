
SRC = cue2pops.c

CC = gcc
CFLAGS = -Wall

TARGET = cue2pops

RM = rm

all: cue2pops

cue2pops:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	$(RM) $(TARGET)
