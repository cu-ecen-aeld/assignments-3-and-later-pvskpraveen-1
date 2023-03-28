

CC=gcc
CROSS=aarch64-none-linux-gnu-
	
writer:
	$(CC) -o writer writer.c

CROSS_COMPILE:
	$(CROSS)$(CC) -o writer writer.c
	
clean:
	rm -f *.o writer