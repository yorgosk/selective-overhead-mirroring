#ifndef __MIRROR_INITIATOR_FUNCTIONS__
#define __MIRROR_INITIATOR_FUNCTIONS__

#include "MirrorInitiator_header.h"

/* Global Variables */
extern hostent* mirrorServerHostEnt;
extern char* mirrorServerAddress;
extern int mirrorServerPort, sock, numOfContentServers, port;
extern sockaddr_in server, client;
extern sockaddr *serverptr, *clientptr;
extern socklen_t clientlen;
extern hostent *rem;

/* take any information provided about the mirror server and properly store it */
void takeMirrorServerInfo(char*);
/* set-up mirror initiator */
void setupMirrorInitiator();
/* shut-down mirror initiator */
void shutdownMirrorInitiator();
/* send content request to mirror server */
void sendContentRequest(char**);

#endif