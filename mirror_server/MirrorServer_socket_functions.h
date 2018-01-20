#ifndef __MIRROR_SERVER_SOCKET_FUNCTIONS__
#define __MIRROR_SERVER_SOCKET_FUNCTIONS__

#include "MirrorServer_header.h"

/* Global Variables */
extern int port, threadnum, sock, newsock, numDevicesDone, numOfRequests, activeWorkers, activeMirrorManagers, 
		mirrorManager, worker, done, bytesTransfered, filesTransfered, minSize, maxSize, range;
extern double averageTransferedSize, averageDeviation, *values;
extern char* dirname;
extern sockaddr_in server, client;
extern sockaddr *serverptr, *clientptr;
extern socklen_t clientlen;
extern pthread_t *workerIDs, *mirrorManagerIDs;
extern requestInfo* request;
/* the queue buffer */
extern buffer_t buffer;
/* mutexes for synchronization */
extern pthread_mutex_t buffer_mtx, gethostbyname_mtx, allDone_mtx;
/* condition variables */
extern pthread_cond_t allDone, cond_buffer_nonempty, cond_buffer_nonfull, buffer_read_cond, buffer_write_cond;
/* set-up mirror server */
void setupMirrorServer();
/* listen for connections */
void mirrorServerListenForConnections();
/* process content request from mirror initiator */
void processContentRequest();
/* take any information provided about the content servers and properly store it */
void takeRequestInfo(char*);
/* shut-down mirror server */
void shutdownMirrorServer();

#endif