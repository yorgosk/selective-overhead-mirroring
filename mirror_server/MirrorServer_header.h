#ifndef __MIRROR_SERVER_HEADER__
#define __MIRROR_SERVER_HEADER__

#define BUFSIZE 2048
#define REQBUFFERSIZE 10
#define perror2(s, e) fprintf(stderr, "%s: %s\n", s, strerror(e))

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
#include <pthread.h>		// threads
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

typedef struct stat Stat;
typedef struct in_addr in_addr;
typedef struct hostent hostent;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

typedef struct requestInfo {
	int id;
	hostent* contentServerHostEnt;
	char* contentServerAddress;
	int contentServerPort;
	sockaddr_in contentServer;
	sockaddr *contentServerPtr;
	int contentServerSock;
	char* dirorfile;
	int delay;
} requestInfo;

typedef struct bufferRec {
	int id;
	char** dirorfilename;
	int numOfContents;
	char* contentServerAddress;
	int contentServerPort;
} bufferRec;

typedef struct buffer_t {
	bufferRec data[REQBUFFERSIZE];	// buffer is static, as specified
	int start;
	int end;
	int count;
} buffer_t;

#include "MirrorServer_socket_functions.h"
#include "MirrorServer_thread_functions.h"

/* Global Variables */
/* for "relative dispersion", source: http://sciencing.com/calculate-dispersion-10018216.html */
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

/* function declarations */
/* initialize queue buffer's members */
void initializeBuffer(buffer_t*);

#endif