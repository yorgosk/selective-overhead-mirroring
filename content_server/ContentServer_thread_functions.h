#ifndef __CONTENT_SERVER_CHILDREN_FUNCTIONS
#define __CONTENT_SERVER_CHILDREN_FUNCTIONS

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

/* our function for the thread that is going to serve a connection */
void* threadFunc(void*);
/* listen command from mirror server */
void listenCommand(int);
/* process command from mirror server */
void processCommand(char*, int);
/* it would be very bad if someone passed us an dirname like
* "; rm *" and we naively created a command " ls ; rm *".
* So .. we remove everything but slashes and alphanumerics. */
void sanitize(char*);

#endif