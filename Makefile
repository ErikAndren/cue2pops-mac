
SRC = cue2pops.c

CC = gcc

TARGET=cue2pops

all:
	$(CC) $(SRC) -o $(TARGET)
