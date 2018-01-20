#ifndef __CONTENT_SERVER_SOCKET_FUNCTIONS__
#define __CONTENT_SERVER_SOCKET_FUNCTIONS__

#include "ContentServer_header.h"

/* Global Variables */
extern int port, sock, delay, numOfRequests, threads;
extern sockaddr_in server, client;
extern sockaddr *serverptr, *clientptr;
extern socklen_t clientlen;
extern char* dirorfilename;
extern request* reqInfo;
extern pthread_t *threadID;

/* set-up content server */
void setupContentServer();
/* listen for connections */
void contentServerListenForConnections();
/* shut-down content server */
void shutdownContentServer();

#endif