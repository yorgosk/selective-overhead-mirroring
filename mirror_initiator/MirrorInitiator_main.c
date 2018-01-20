#include "MirrorInitiator_header.h"

/* Global Variables */
hostent* mirrorServerHostEnt;
char* mirrorServerAddress;
int mirrorServerPort, sock, numOfContentServers, port;
sockaddr_in server, client;
sockaddr *serverptr, *clientptr;
socklen_t clientlen;
hostent *rem;

int main(int argc, char* argv[]) {
	printf("Welcome to the Mirror Initiator!\n");
	/* SET UP BASED ON COMMAND LINE'S ARGUMENTS */
	if (argc < 7) {     // check number of arguments
        fprintf(stderr, "ERROR! Insufficient number of arguments!\n");
        exit(EXIT_FAILURE);
    }
    // initialize variables
    int i, nflag = 0, pflag = 0, sflag = 0;
    char **contentServerStr = NULL;
    numOfContentServers = 0;
    mirrorServerAddress = NULL;

    for(i = 1; i < argc; i++) {
        // printf("%d\n", i);  // for debugging
        if(!strcmp(argv[i], "-n") && !nflag && strcmp(argv[i+1], "-p") != 0 && strcmp(argv[i+1], "-s") != 0) {
            // take and store mirror server address info
            takeMirrorServerInfo(argv[i+1]);
            i++;    // we "consumed" another argument
            nflag = 1;
        } else if(!strcmp(argv[i], "-p") && !pflag && strcmp(argv[i+1], "-n") != 0 && strcmp(argv[i+1], "-s") != 0) {
            // take and store mirror server port
            mirrorServerPort = atoi(argv[i+1]);
            i++;    // we "consumed" another argument
            pflag = 1;
        } else if(!strcmp(argv[i], "-s") && !sflag && strcmp(argv[i+1], "-n") != 0 && strcmp(argv[i+1], "-p") != 0) {
            sflag = 1;
            i++;    // we "consumed" another argument
            while(i < argc) {
                numOfContentServers++;
                contentServerStr = realloc(contentServerStr, numOfContentServers*sizeof(char*));
                contentServerStr[numOfContentServers-1] = malloc(strlen(argv[i])+1);
                strcpy(contentServerStr[numOfContentServers-1], argv[i]);
                i++;    // we "consumed" another argument
            }
        } else {
            fprintf(stderr, "ERROR! Bad argument formation!\n");
            exit(EXIT_FAILURE);
        }
    }

    if(!nflag || !pflag || !sflag) {
        fprintf(stderr, "ERROR! Bad argument formation!\n");
        exit(EXIT_FAILURE);
    }

    printf("Mirror Initiator: mirrorServerAddress: %s, mirrorServerPort: %d\n", mirrorServerAddress, mirrorServerPort);

    /* set-up mirror initiator */
    setupMirrorInitiator();   
    /* send content request to mirror server */
    sendContentRequest(contentServerStr);
    /* shut-down mirror initiator */
    shutdownMirrorInitiator();

    /* tree dynamically-allocated memory */
    free(mirrorServerAddress);
    for(i = 0; i < numOfContentServers; i++) {
        // printf("contentServerStr[%d]: %s\n", i, contentServerStr[i]);
        free(contentServerStr[i]);
    }
    free(contentServerStr);
    
    printf("Mirror Initiator terminating...\nGoodbye!\n");

	return 0;
}