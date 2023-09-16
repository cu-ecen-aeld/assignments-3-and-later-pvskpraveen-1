
TARGET ?= aesdsocket
SRCS ?= aesdsocket.c
OBJS = $(SRCS: .c=.o)
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror
LDFLAGS ?= -lm -lpthread -lrt



all: 	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $@.o $(LDFLAGS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@.o $(LDFLAGS)
	
clean:
	rm -f $(OBJS) $(TARGET)
