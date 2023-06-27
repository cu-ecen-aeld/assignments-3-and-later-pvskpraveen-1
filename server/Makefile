
CC = gcc
CDEFS = -Wall -Werror
CFLAGS = -g $(CDEFS)

all: 	aesdsocket

aesdsocket: aesdsocket.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o aesdsocket aesdsocket.c
	
clean:
	rm -f *.o *.d
	rm -f aesdsocket
	
.c.o:
	$(CC) $(CFLAGS) -c $<