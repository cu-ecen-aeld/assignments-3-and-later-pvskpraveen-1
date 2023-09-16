/****************************************************************************************************************************************/
/*		purpose: Opens socket and listens on localhost port 9000																			  */
/*																																										  */
/*																																										  */
/*																																										  */
/*																																										  */
/****************************************************************************************************************************************/
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
#include <pthread.h>

#include "queue.h"

#define PORT "9000"
#define BACKLOG 10
#define RECFILE "/var/tmp/aesdsocketdata"
#define BUFLEN 512
#define TIMESTAMPLEN 32

bool process_killed=false;
bool timer_elapsed = false;

/*Signal Handler for to catch SIGINT or SIGTERM*/
static void signal_handler(int signal_number)	{
	syslog(LOG_INFO, "Caught signal, exiting");
	process_killed = true;
}

static void timer_handler(int signal_number)	{
	syslog(LOG_INFO, "Timer expired");
	timer_elapsed = true;
}

typedef struct slist_data_s {
	pthread_t *threadp;
	SLIST_ENTRY(slist_data_s) entries;
}slist_data_t;

struct threadparam_t {
	pthread_mutex_t *mutexp;
	char ipstr[INET_ADDRSTRLEN];
	int listen_fd;
	bool thread_complete_success;
};

void timer_function(pthread_mutex_t * mutexp) {
	int recfile_fd;
	time_t curtime;
	char *timestamp; 
	
	timestamp = (char *)calloc(TIMESTAMPLEN+1, sizeof(char));
	
	while(!process_killed)	{
		alarm(10);
		pause();
		if (timer_elapsed)	{
			time(&curtime);
			strftime(timestamp, TIMESTAMPLEN, "timestamp: %Y %m %d %H:%M:%S\n", localtime(&curtime));
			syslog(LOG_INFO, "%s", timestamp);
			pthread_mutex_lock(mutexp);
			recfile_fd = open(RECFILE, O_CREAT | O_RDWR | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
			if (recfile_fd == -1) {
				perror("Error: ");
				return;
			}
			write(recfile_fd, timestamp, strlen(timestamp));
			close(recfile_fd);
			
			pthread_mutex_unlock(mutexp);
			
			timer_elapsed = false;
		}
	}
	
	free(timestamp);
}

/*Thread handler for writing to file*/
 void *socket_handler(void *threadparamp)	{
		ssize_t datalen;
		char filebuf[BUFLEN], recbuf[BUFLEN];
		char *sockbuf;
		int recfile_fd;
		struct threadparam_t* thread_func_args = (struct threadparam_t *) threadparamp;
		thread_func_args->thread_complete_success = false;
		
		sockbuf = (char *)calloc(BUFLEN+1, sizeof(char));
		
		syslog(LOG_INFO, "Accepted connection from %s", thread_func_args->ipstr);
		
		datalen = recv(thread_func_args->listen_fd, recbuf, BUFLEN, 0);
				
		syslog(LOG_INFO,"Check for Mutex availability \n");
		pthread_mutex_lock(thread_func_args->mutexp);			// Lock Mutex
		syslog(LOG_INFO,"Mutex locked \n");
		/*Open file to write the received data*/
		
		recfile_fd = open(RECFILE, O_CREAT | O_RDWR | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
		if (recfile_fd == -1) {
			perror("Error: ");
			return (void *) thread_func_args;
		}
	
		if (datalen == -1)
			perror("RECV Error: ");
		strncpy(sockbuf, recbuf, datalen);
		while (datalen > 0)	{
				if (datalen == BUFLEN && recbuf[datalen-1] != '\n') {
					sockbuf = realloc(sockbuf, (strlen(sockbuf)+BUFLEN)*sizeof(char) + 1);
					datalen = recv(thread_func_args->listen_fd, recbuf, datalen, 0);
					strncat(sockbuf, recbuf, datalen);
				}
				else
					break;
		}
		syslog(LOG_INFO, "%s", sockbuf);
		write(recfile_fd, sockbuf, strlen(sockbuf));
		close(recfile_fd);
		
		recfile_fd = open(RECFILE, O_RDONLY);
		if (recfile_fd == -1) {
			perror("Error: ");
			return (void *) thread_func_args;
		}
		
		while (1) {
			datalen = read(recfile_fd, filebuf, BUFLEN);
			if (datalen == 0) break;
			send(thread_func_args->listen_fd, filebuf, datalen, 0);
		} 
				
				/*Close the file*/
		close(recfile_fd);
		pthread_mutex_unlock(thread_func_args->mutexp);			// Unlock Mutex
		syslog(LOG_INFO,"Mutex unlocked \n");
		free(sockbuf);
		
		syslog(LOG_INFO, "Closed connection from %s", thread_func_args->ipstr);
		thread_func_args->thread_complete_success = true;
		
		return (void *) thread_func_args;
}

/**/
int main(int argc, char* argv[])	 {
	int status;
	int listen_fd;
	struct addrinfo hints;
	struct sockaddr_storage their_addr;
	int addrlen = sizeof(struct sockaddr_storage);
	struct sigaction kill_action;
	struct sigaction alrm_action;
	struct addrinfo *res;
	int socket_fd;
	pid_t procid;
	struct threadparam_t* threadparamp;
	pthread_t thread;
	
	char ipstr[INET_ADDRSTRLEN];
	
	//char *recdata;
	
	/*Mutex*/
	pthread_mutex_t mutex;
	if(pthread_mutex_init(&mutex, NULL)) {
		perror("Unable to initiate Mutex");
		return -1;
	}
	
	/*Linked list handling*/
	slist_data_t *thread_list_p=NULL;
	
	SLIST_HEAD(slisthead, slist_data_s)	head;
	SLIST_INIT(&head);
	
	/*Signal*/
	memset(&kill_action, 0, sizeof(struct sigaction));
	kill_action.sa_handler=signal_handler;
	memset(&alrm_action, 0, sizeof(struct sigaction));
	alrm_action.sa_handler=timer_handler;
	
	if(0!=sigaction(SIGTERM, &kill_action, NULL))
		perror("error registering for SIGTERM\n");
	if(0!=sigaction(SIGINT, &kill_action, NULL))
		perror("error registering for SIGINT\n");
	if(0!=sigaction(SIGALRM, &alrm_action, NULL))
		perror("error registering for SIGALRM\n");
	
		
	/*Socket initialization*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0)	 {
		perror("getaddrinfo");
		return -1;
	}
	
	if ((socket_fd = socket(res->ai_family, res->ai_socktype, res-> ai_protocol)) == -1)	{
		freeaddrinfo(res);
		perror("Socket");
		return -1;
	}
	
	if(0!=bind(socket_fd, res->ai_addr, res->ai_addrlen))	{
		freeaddrinfo(res);
		perror("bind");
		return -1;
	}
	
	freeaddrinfo(res);
		
	if ((argc == 2) && (strcmp(argv[1], "-d")==0))  {
		procid = fork();
		if (procid > 0) { //parent 
			printf("Initiated Daemon mode, Parent exiting...\n");
			return 0;	//parent process ends to set the Child process to be a daemon
		}
	}
	if(fork())	{
		timer_function(&mutex);
		return 0;
	}
		
	listen(socket_fd, BACKLOG);
	printf("waiting\n");
	
	
	while(!process_killed) {	// Keep listening until the proces is killed
		listen_fd = accept(socket_fd, (struct sockaddr *)&their_addr, (socklen_t*)&addrlen);
		if (listen_fd == -1) {
			break;
		}
		inet_ntop(AF_INET, &their_addr, ipstr, INET_ADDRSTRLEN);
		
		threadparamp = (struct threadparam_t *)malloc(sizeof(struct threadparam_t));
		threadparamp->mutexp = &mutex;
		threadparamp->listen_fd = listen_fd;
		memcpy(threadparamp->ipstr, ipstr, INET_ADDRSTRLEN * sizeof(char));
		
		/*create a new thread when a new connection is received*/
		/*Add the thread id to the linked list*/
		if(0==pthread_create(&thread, // thread id passed as input
									(void*) 0,	// no atributes required
									socket_handler,	// Thread handler
									(void*) threadparamp	// arguments for the thread
									))	{
			thread_list_p = malloc(sizeof(slist_data_t));
			thread_list_p->threadp = &thread;
			SLIST_INSERT_HEAD(&head, thread_list_p, entries) ;
		}
	} 
	
	
	SLIST_FOREACH(thread_list_p, &head, entries) {
		pthread_join(*thread_list_p->threadp, (void *)threadparamp);
		SLIST_REMOVE(&head, thread_list_p, slist_data_s, entries);
	}
	
	free(threadparamp);
	free(thread_list_p);
	
	pthread_mutex_destroy(&mutex);
	if(listen_fd != -1) close(listen_fd);
	if(socket_fd != -1) close(socket_fd);	// closes socket and frees up resources associated with the socket
	remove(RECFILE);
			
	return 0;
}