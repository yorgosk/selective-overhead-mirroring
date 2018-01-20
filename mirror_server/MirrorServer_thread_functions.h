#ifndef __MIRROR_SERVER_THREAD_FUNCTIONS__
#define __MIRROR_SERVER_THREAD_FUNCTIONS__

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
/* create worker threads */
void createWorkerThreads();
/* wait for worker threads' termination  */
void waitWorkerThreads();
/* create mirror manager threads */
void createMirrorManagerThreads();
/* wait for mirror manager threads' termination */
void waitMirrorManagerThreads();
/* mirror manager thread which communicates with a content server */
void *mirrorManagerThread(void*);
/* worker thread which fetches files from a content server */
void *workerThread(void*);
/* worker thread fetches a file from content server */
void workerFetch();
/* check if we have a file or a directory */
/* source: https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file */
int isDirectory(const char*);
/* make path's directories or subdirectories if needed */
/* source: https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix */
void my_mkdir(const char*, int, mode_t);
/* check prefix */
/* source: https://stackoverflow.com/questions/4770985/how-to-check-if-a-string-starts-with-another-string-in-c */
int startsWith(const char*, const char *);

#endif