/******************************************************************
*		name: writer.c
*		auther: pvskpraveen
*
*		expected inout argument 1 = File name with path
*		argument 2 = string to be written
*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int fd;
	
	openlog(NULL, 0, LOG_USER);
	
	if (argc != 3) {
		syslog(LOG_ERR, "Incorrect arguments provided, %d ", argc );
		return 1;
	}
	
	fd = open( argv[1] , O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
	if ( fd == -1) {
		syslog(LOG_ERR,  "Unable to open file" );
		return 1;
	}
	syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1] );
	write(fd, argv[2], strlen(argv[2]));
	
	close(fd);
	return 0;
}