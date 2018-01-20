#ifndef __MIRROR_INITIATOR_HEADER__
#define __MIRROR_INITIATOR_HEADER__

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

typedef struct in_addr in_addr;
typedef struct hostent hostent;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

#include "MirrorInitiator_functions.h"

#define BUFSIZE 2048

/* Global Variables */
extern hostent* mirrorServerHostEnt;
extern char* mirrorServerAddress;
extern int mirrorServerPort, sock, numOfContentServers, port;
extern sockaddr_in server, client;
extern sockaddr *serverptr, *clientptr;
extern socklen_t clientlen;
extern hostent *rem;

#endif