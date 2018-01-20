#include "MirrorServer_socket_functions.h"

void setupMirrorServer() {
	serverptr = (sockaddr *)&server;
    /* create socket */
    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    server.sin_family = AF_INET;    // internet domain
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);  // the given port
    /* Bind socket to address */
    /* source: https://stackoverflow.com/questions/22489981/linux-socket-client-server-program?rq=1 */
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(bind(sock, serverptr, sizeof(server)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    /* create our directory */
	if((mkdir(dirname, S_IRWXU)) != 0) {
		perror("mkdir\n");
		exit(EXIT_FAILURE);
	}
    /* create worker threads */
    createWorkerThreads();
}

void mirrorServerListenForConnections() {
	clientptr = (sockaddr *)&client;
    /* listen for connections */
    if(listen(sock, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Mirror Server: Listening for connections to port %d\n", port);
    /* accept connection */
    if((newsock = accept(sock, clientptr, &clientlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("Accepted connection\n");
    /* process content request from mirror initiator */
    processContentRequest();
    /* create mirror manager threads */
	createMirrorManagerThreads();
}

void processContentRequest() {
	char buf[BUFSIZE];
	/* receive request */
	while(read(newsock, buf, BUFSIZE) > 0 && strcmp(buf, "done") != 0) {
		if(!strcmp(buf, "")) continue;	// we don't want to read an empty string
		/* print received string */
		// printf("buf = \"%s\"\n", buf);
		/* take any information provided about the content servers and properly store it */
		takeRequestInfo(buf);
		/* reply "ok" */
		strcpy(buf, "ok");
		while(write(newsock, buf, BUFSIZE) < 0);
	}
}

void takeRequestInfo(char* contentServerStr) {
	// printf("%s\n", contentServerStr);
	/* we have one more content request */
	numOfRequests++;
	request = realloc(request, numOfRequests*sizeof(requestInfo));
	/* note request's/content-server's ID */
	request[numOfRequests-1].id = numOfRequests;
	char *tok, *temp = NULL;
	/* store our complete string of content-server-info temporarily */
	temp = malloc(strlen(contentServerStr)+1);
	strcpy(temp, contentServerStr);
	/* get content server-host by name */
	tok = strtok(temp, ":");
	if((request[numOfRequests-1].contentServerHostEnt = gethostbyname(tok)) == NULL) {
		perror("gethostbyname");
		exit(EXIT_FAILURE);
	}
	/* resolve content server's address */
	in_addr **addr_list;
	addr_list = (in_addr**) request[numOfRequests-1].contentServerHostEnt->h_addr_list;
	request[numOfRequests-1].contentServerAddress = malloc(strlen(inet_ntoa(*addr_list[0]))+1);
	strcpy(request[numOfRequests-1].contentServerAddress, inet_ntoa(*addr_list[0]));
	printf("%s -> %s resolved to %s\n", tok, request[numOfRequests-1].contentServerHostEnt->h_name, request[numOfRequests-1].contentServerAddress);
	/* take content server port */
	tok = strtok(NULL, ":");
	request[numOfRequests-1].contentServerPort = atoi(tok);
	/* initialize the socket-to-content-server field */
	request[numOfRequests-1].contentServerSock = -1;
	/* take requested directory or file name */
	tok = strtok(NULL, ":");
	request[numOfRequests-1].dirorfile = malloc(strlen(tok)+1);
	// *(request[numOfRequests-1].dirorfile) = (char){0};
	strcpy(request[numOfRequests-1].dirorfile, tok);
	/* take desired time of delay */
	tok = strtok(NULL, " ,");
	request[numOfRequests-1].delay = atoi(tok);

	free(temp);
}

void shutdownMirrorServer() {
    /* wait for worker threads' termination */
    waitWorkerThreads();
    /* wait for mirror manager threads' termination */
    waitMirrorManagerThreads();
    /* calculate remaining statistics */
    range = maxSize - minSize;
    averageTransferedSize = bytesTransfered/filesTransfered;
    // calculate average deviation
    int i, sum = 0;
    for(i = 0; i < filesTransfered; i++) {
    	values[i] -= averageTransferedSize;
    	sum += values[i];
    }
    averageDeviation = sum/filesTransfered;
    printf("Mirror Server statistics:\ntransfered %dB, %d files, %dB minimum, %dB maximum, range: %d, avg size: %f, avg deviation: %f\n", 
    	bytesTransfered, filesTransfered, minSize, maxSize, range, averageTransferedSize, averageDeviation);

    /* contact mirror initiator for statistics */
    char buf[BUFSIZE];
    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    memset(buf, '\0', BUFSIZE);
    // formulate statistics' response to mirror initiator
	snprintf(buf, BUFSIZE, "Mirror Server statistics:\ntransfered %dB, %d files, %dB minimum, %dB maximum, range: %d, avg size: %f, avg deviation: %f\n", 
    	bytesTransfered, filesTransfered, minSize, maxSize, range, averageTransferedSize, averageDeviation);

	/* inform about statistics */
       	while(write(newsock, "statistics", strlen("statistics")) < 0);

    /* send statistics */
    while(write(newsock, buf, BUFSIZE) < 0);

	/* close sockets */
	close(sock);	// close server's socket
	shutdown(newsock, 1);	// Further sends are disallowed
}
