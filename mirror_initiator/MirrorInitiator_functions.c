#include "MirrorInitiator_functions.h"

void takeMirrorServerInfo(char* name) {
	if((mirrorServerHostEnt = gethostbyname(name)) == NULL) {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    in_addr **addr_list;

    addr_list = (in_addr**) mirrorServerHostEnt->h_addr_list;
    mirrorServerAddress = malloc(strlen(inet_ntoa(*addr_list[0]))+1);
    strcpy(mirrorServerAddress, inet_ntoa(*addr_list[0]));
    printf("%s -> %s resolved to %s\n", name, mirrorServerHostEnt->h_name, mirrorServerAddress);
}

void setupMirrorInitiator() {
	serverptr = (sockaddr*)&server;
    /* create socket */
    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    /* find server address */
    if((rem = gethostbyname(mirrorServerAddress)) == NULL) {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }
    server.sin_family = AF_INET;    // Internet domain
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    printf("memcpy\n");
    server.sin_port = htons(mirrorServerPort);  // server port
    /* initiate connection */
    if(connect(sock, serverptr, sizeof(server)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    printf("Connecting to %s port %d\n", mirrorServerAddress, mirrorServerPort);
}

void sendContentRequest(char** contentServerStr) {
    char buf[BUFSIZE];
    int i;
    for(i = 0; i < numOfContentServers; i++) {
        // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
        memset(buf, '\0', BUFSIZE);
        strcpy(buf, contentServerStr[i]);
        // printf("sending %s\n", buf);
        /* send i-th mirro server */
        while(write(sock, buf, BUFSIZE) < 0);
        /* receive "ok" */
        while(read(sock, buf, BUFSIZE) < 0 || strcmp(buf, "ok") != 0);
        // printf("Received string: %s\n", buf);
    }

    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    memset(buf, '\0', BUFSIZE);
    strcpy(buf, "done");
    /* send "done" to mirror server */
    while(write(sock, buf, BUFSIZE) < 0);
    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    memset(buf, '\0', BUFSIZE);

	/* wait for statistics message */
	while(read(sock, buf, BUFSIZE) < 0 || strcmp(buf, "statistics") != 0);
	printf("%s\n", buf);
    /* receive statistics */
    while(read(sock, buf, BUFSIZE) < 0);

    printf("Received statistics: %s\n", buf);
}

void shutdownMirrorInitiator() {
    /* close sockets */
	close(sock);
}
