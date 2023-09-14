INCLUDE_DIRS = 
LIB_DIRS = 
CC = gcc

CDEFS = -Wall -Werror
CFLAGS = -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS = -lm -lpthread

HFILES = 
CFILES = aesdsocket.c

SRCS = $(HFILES) $(CFILES)
OBJS = $(CFILES: .c=.o)

all: 	aesdsocket

aesdsocket: aesdsocket.o
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o $@ $@.o $(LIBS)
	
clean:
	rm -f *.o *.d
	rm -f aesdsocket
	
.c.o:
	$(CC) $(CFLAGS) -c $<