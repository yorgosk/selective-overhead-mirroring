#ifndef __CONTENT_SERVER_COMMANDS__
#define __CONTENT_SERVER_COMMANDS__

#include "ContentServer_header.h"

/* Global Variables */
extern int port, sock, delay, numOfRequests, threads;
extern sockaddr_in server, client;
extern sockaddr *serverptr, *clientptr;
extern socklen_t clientlen;
extern FILE* sock_fp;	// stream for socket IO
extern char* dirorfilename;
extern request* reqInfo;
extern pthread_t *threadID;

/* inform mirror server of the files this content server contains, while getting informed of the delay under which it is going to send results */
void list(char*, int);
/* fetch the contents of a specific directory or file name (if it exists), after waiting for "delay" seconds */
void fetch(char*, int);

#endif