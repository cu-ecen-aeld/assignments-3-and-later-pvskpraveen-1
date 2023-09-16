
SRCS ?= aesdsocket.c
OBJS ?= aesdsocket.o
TARGET ?= aesdsocket
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror
LDFLAGS ?= -lm -lpthread -lrt

all: 	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $@.o $(LDFLAGS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)
	
clean:
	rm -f *.o $(TARGET)
