#ifndef __CONTENT_SERVER_HEADER__
#define __CONTENT_SERVER_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>		// sockets
#include <sys/socket.h>		// sockets
#include <sys/types.h>		// sockets
#include <netinet/in.h>		// internet sockets
#include <netdb.h>			// gethostbyaddr
#include <ctype.h>			// exit
#include <signal.h>			// signal
#include <unistd.h>			// close
#include <arpa/inet.h>
#include <signal.h>			// signal
#include <sys/stat.h>
#include <pthread.h>

#define BUFSIZE 2048
#define perror2(s, e) fprintf(stderr, "%s: %s\n", s, strerror(e))

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

typedef struct request{
	int contentServerID;
	int delay;
} request;

#include "ContentServer_socket_functions.h"
#include "ContentServer_thread_functions.h"
#include "ContentServer_commands.h"

/* Global Variables */
extern int port, sock, delay, numOfRequests, threads;
extern sockaddr_in server, client;
extern sockaddr *serverptr, *clientptr;
extern socklen_t clientlen;
extern char* dirorfilename;
extern request* reqInfo;
extern pthread_t *threadID;

/* Various functions */
/* handler for termination signal */
void terminate(int);

#endif