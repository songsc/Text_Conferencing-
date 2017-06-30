
CC := gcc
CFLAGS = -g -Wall 
LDFLAGS= -pthread -lpthread -lrt
RM := rm -rf
TARGETS = server client
SRCS = server.c client.c utils.c server_lib.c client_lib.c

.PHONY: default all clean

default: all
all: $(TARGETS)

clean:
	$(RM) $(TARGETS) $(SRCS:.c=.o)

server: server.o utils.o server_lib.o

client: client.o utils.o client_lib.o

depend:
	$(CC) -MM *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif