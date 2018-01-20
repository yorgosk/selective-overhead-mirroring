#include "ContentServer_header.h"

/* Global Variables */
int port, sock, delay, numOfRequests, threads;
sockaddr_in server, client;
sockaddr *serverptr, *clientptr;
socklen_t clientlen;
char* dirorfilename;
request* reqInfo;
pthread_t *threadID;

int main(int argc, char* argv[]) {
	printf("Welcome to the Content Server!\n");
    /* SET HANDLER FOR TERMINATION SIGNAL */
    signal(SIGINT, terminate);
	/* SET UP BASED ON COMMAND LINE'S ARGUMENTS */
	if (argc < 5) {     // check number of arguments
        fprintf(stderr, "ERROR! Insufficient number of arguments!\n");
        exit(EXIT_FAILURE);
    }
    // initialize variables
    int pflag = 0, dflag = 0, i;
    dirorfilename = NULL;
    numOfRequests = 0;
    reqInfo = NULL;
    threads = 0;
    threadID = NULL;

    for(i = 1; i < argc-1; i += 2) {    // match variables with parameters' values
    	if(!pflag && !strcmp(argv[i], "-p")) {
    		pflag = 1;
    		port = atoi(argv[i+1]);
    	} else if(!dflag && !strcmp(argv[i], "-d")) {
    		dflag = 1;
    		dirorfilename = malloc(strlen(argv[i+1])+1);
    		strcpy(dirorfilename, argv[i+1]);
    	} else {
            fprintf(stderr, "ERROR! Bad argument formation!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!pflag || !dflag) {   // test if all the neccessary flags have been included
        fprintf(stderr, "ERROR! Arguments missing!\n");
        exit(EXIT_FAILURE);
    }

    printf("Content Server: port: %d, dirorfilename: %s\n", port, dirorfilename);

    /* set-up content server */
    setupContentServer();
    /* listen for connections */
    contentServerListenForConnections();
    /* shut-down content server */
    shutdownContentServer();

    /* free dynamically-allocated memory */
    free(dirorfilename);
    free(reqInfo);
    free(threadID);

    printf("Content Server terminating...\nGoodbye!\n");

	return 0;
}

void terminate(int signum) {
    signal(SIGINT, terminate);     // re-establish disposition of the signal SIGTERM
    /* shut-down content server */
    shutdownContentServer();
    /* free dynamically-allocated memory */
    free(dirorfilename);

    printf("\nContent Server terminating...\nGoodbye!\n");

    exit(EXIT_SUCCESS);
}