
SRC = cue2pops.c

CC = gcc

TARGET = cue2pops

RM = rm

all: cue2pops

cue2pops:
	$(CC) $(SRC) -o $(TARGET)

clean:
	$(RM) $(TARGET)
