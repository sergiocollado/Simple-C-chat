CC = gcc
CFLAGS = -w -Wextra -Wall
LDFLAGS = -pthread -lnsl -lrt

all: chatserver chatclient

chatserver: chatserver.c nethelp.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ chatserver.c nethelp.c

chatclient: chatclient.c nethelp.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ chatclient.c nethelp.c

clean: 
	rm -f chatserver chatclient *.o core 
