#include "MirrorServer_header.h"

/* Global Variables */
int port, threadnum, sock, newsock, numDevicesDone, numOfRequests, numOfServedRequests, activeWorkers, activeMirrorManagers, 
    mirrorManager, worker, done, bytesTransfered, filesTransfered, minSize, maxSize, range;
double averageTransferedSize, averageDeviation, *values;
char* dirname;
sockaddr_in server, client;
sockaddr *serverptr, *clientptr;
socklen_t clientlen;
requestInfo* request;
pthread_t *workerIDs, *mirrorManagerIDs;
/* the queue buffer */
buffer_t buffer;
/* mutexes for synchronization */
pthread_mutex_t buffer_mtx, gethostbyname_mtx, allDone_mtx;
/* condition variables */
pthread_cond_t allDone, cond_buffer_nonempty, cond_buffer_nonfull, buffer_write_cond, buffer_read_cond;

int main(int argc, char* argv[]) {
	printf("Welcome to the Mirror Server!\n");
	/* SET UP BASED ON COMMAND LINE'S ARGUMENTS */
	if (argc < 7) {     // check number of arguments
        fprintf(stderr, "ERROR! Insufficient number of arguments!\n");
        exit(EXIT_FAILURE);
    }
    /* initialize variables */
    int pflag = 0, mflag = 0, wflag = 0, i;
    numOfRequests = 0;          // we don't have any requests yet
    numDevicesDone = 0;         // no devices done so far
    activeWorkers = 0;
    activeMirrorManagers = 0;
    mirrorManager = 0;
    worker = 0;
    done = 0;
    dirname = NULL;
    request = NULL;
    bytesTransfered = 0;
    filesTransfered = 0;
    minSize = BUFSIZE;
    maxSize = -1;
    range = 0;
    averageTransferedSize = 0;
    averageDeviation = 0;
    values = NULL;
    /* initialize queue buffer */
    initializeBuffer(&buffer);
    /* initialize mutexes */
    pthread_mutex_init(&buffer_mtx, 0);
    pthread_mutex_init(&gethostbyname_mtx, 0);
    pthread_mutex_init(&allDone_mtx, 0);
    /* initialize condition variables */
    pthread_cond_init(&allDone, 0);
    pthread_cond_init(&cond_buffer_nonempty, 0);
    pthread_cond_init(&cond_buffer_nonfull, 0);
    pthread_cond_init(&buffer_read_cond, 0);
    pthread_cond_init(&buffer_write_cond, 0);

	for(i = 1; i < argc-1; i += 2) {    // match variables with parameters' values
    	if(!pflag && !strcmp(argv[i], "-p")) {
    		pflag = 1;
    		port = atoi(argv[i+1]);
    	} else if(!mflag && !strcmp(argv[i], "-m")) {
    		mflag = 1;
    		dirname = malloc(strlen(argv[i+1])+1);
    		strcpy(dirname, argv[i+1]);
    	} else if(!wflag && !strcmp(argv[i], "-w")) {
    		wflag = 1;
    		threadnum = atoi(argv[i+1]);
    	} else {
            fprintf(stderr, "ERROR! Bad argument formation!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!pflag || !mflag || !wflag) {   // test if all the neccessary flags have been included
        fprintf(stderr, "ERROR! Arguments missing!\n");
        exit(EXIT_FAILURE);
    }

    printf("Mirror Server: port: %d, dirname: %s, threadnum: %d\n", port, dirname, threadnum);

    /* set-up mirror server */
    setupMirrorServer();
    /* listen for connections */
    mirrorServerListenForConnections();
    /* shut-down mirror server */
    shutdownMirrorServer();

    /* free dynamically-allocated memory */
    free(dirname);
    for(i = 0; i < numOfRequests; i++) {
        // printf("contentServerAddress: %s, contentServerPort: %d, dirorfile: %s, delay: %d\n",
            // request[i].contentServerAddress, request[i].contentServerPort, request[i].dirorfile, request[i].delay);
        free(request[i].contentServerAddress);
        free(request[i].dirorfile);
    }
    free(request);
    /* clear buffer's records */
    for(i = buffer.start; i <= buffer.end; i++) {
        free(buffer.data[i].contentServerAddress);
        free(buffer.data[i].dirorfilename);
    }
    free(values);
    /* destroy mutexes */
    pthread_mutex_destroy(&buffer_mtx);
    pthread_mutex_destroy(&gethostbyname_mtx);
    pthread_mutex_destroy(&allDone_mtx);
    /* destroy condition variables */
    pthread_cond_destroy(&allDone);
    pthread_cond_destroy(&cond_buffer_nonempty);
    pthread_cond_destroy(&cond_buffer_nonfull);
    pthread_cond_destroy(&buffer_read_cond);
    pthread_cond_destroy(&buffer_write_cond);

    printf("Mirror Server terminating...\nGoodbye!\n");

	return 0;
}

/* initialize queue buffer's members */
void initializeBuffer(buffer_t* buffer) {
    buffer->start = 0;
    buffer->end = -1;
    buffer->count = 0;
}