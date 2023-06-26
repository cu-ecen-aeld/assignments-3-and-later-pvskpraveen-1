#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#define PORT "9000"
#define BACKLOG 10
#define RECFILE "/var/tmp/aesdsocketdata"
#define BUFLEN 512

struct addrinfo *res;
int socket_fd;
int recfile_fd;
bool process_killed=false;

/*Signal Handler for to catch SIGINT or SIGTERM*/
static void signal_handler(int signal_number)
{
	syslog(LOG_INFO, "Caught signal, exiting");
	process_killed = true;
}


int main(int argc, char* argv[])
{
	int status;
	int listen_fd;
	ssize_t datalen;
	struct addrinfo hints;
	struct sockaddr_storage their_addr;
	int addrlen = sizeof(struct sockaddr_storage);
	struct sigaction new_action;
	pid_t procid;
	char *sockbuf;
	sockbuf = (char *)malloc(BUFLEN* sizeof(char));
	
	char ipstr[INET_ADDRSTRLEN];
	
	char filebuf[BUFLEN], recbuf[BUFLEN];
	//char *recdata;
	
	memset(&new_action, 0, sizeof(struct sigaction));
	new_action.sa_handler=signal_handler;
	if(0!=sigaction(SIGTERM, &new_action, NULL))
		perror("error registering for SIGTERM\n");
	if(0!=sigaction(SIGINT, &new_action, NULL))
		perror("error registering for SIGTERM\n");
		
	/******
	Socket initialization
	******/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0)
	{
		return -1;
	}
	
	socket_fd = socket(res->ai_family, res->ai_socktype, res-> ai_protocol);
	if (-1==socket_fd) {
		perror("Socket Error");
		freeaddrinfo(res);
		return -1;
	}
	if(0!=bind(socket_fd, res->ai_addr, res->ai_addrlen))	{
		perror("Bind Error");
		freeaddrinfo(res);
		return -1;
	}
	
	freeaddrinfo(res);
		
	if ((argc == 2) && (strcmp(argv[1], "-d")==0))
	{
		procid = fork();
		if (procid > 0) //parent
		{
			printf("Initiated Daemon mode, Parent exiting...\n");
			return 0;	//parent process ends to set the Child process to be a daemon
		}
	}
		
	listen(socket_fd, BACKLOG);
	
	while(!process_killed)
	{	
		listen_fd = accept(socket_fd, (struct sockaddr *)&their_addr, (socklen_t*)&addrlen);
		if (listen_fd == -1) {
			break;
		}
		inet_ntop(AF_INET, &their_addr, ipstr, INET_ADDRSTRLEN);
		syslog(LOG_INFO, "Accepted connection from %s", ipstr);
			
		/*Open file to write the received data*/
		recfile_fd = open(RECFILE, O_CREAT | O_RDWR | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
		if (recfile_fd == -1) {
			perror("Error: ");
			break;
		}
		
		datalen = recv(listen_fd, recbuf, BUFLEN, 0);
		
		if (datalen == -1)
			perror("RECV Error: ");
		strncpy(sockbuf, recbuf,datalen);
		while (datalen > 0)
		{
				if (datalen == BUFLEN && recbuf[datalen-1] != '\n') {
					sockbuf = realloc(sockbuf, (strlen(sockbuf)+BUFLEN)*sizeof(char));
					datalen = recv(listen_fd, recbuf, datalen, 0);
					strncat(sockbuf, recbuf, datalen);
				}
				else
					break;
		}
		write(recfile_fd, sockbuf, strlen(sockbuf));
		close(recfile_fd);
		
		recfile_fd = open(RECFILE, O_RDONLY);
		if (recfile_fd == -1) {
			perror("Error: ");
			return -1;
		}
		
		while (1) {
			datalen = read(recfile_fd, filebuf, BUFLEN);
			if (datalen == 0) break;
			send(listen_fd, filebuf, datalen, 0);
		} 
		close(recfile_fd);
		syslog(LOG_INFO, "Closed connection from %s", ipstr);
		
	} 
	free(sockbuf);
	if(listen_fd != -1) close(listen_fd);
	if(socket_fd != -1) close(socket_fd);	// closes socket and frees up resources associated with the socket
	if(recfile_fd != -1) close(recfile_fd);
	remove(RECFILE);
	
	return 0;
}