#include "ContentServer_socket_functions.h"

void setupContentServer() {
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
}

void contentServerListenForConnections() {
    /* listen for connections */
    if(listen(sock, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Mirror Server: Listening for connections to port %d\n", port);
    while(1) {
    	int newsock;
    	/* accept connection */
	    if((newsock = accept(sock, clientptr, &clientlen)) < 0) {
	        perror("accept");
	        exit(EXIT_FAILURE);
	    }
	    printf("Accepted connection\n");
	    /* create thread for serving client */
	    int err;
	    threads++;
	    threadID = realloc(threadID, threads*sizeof(pthread_t));
	    /* create a thread */
		if((err = pthread_create(threadID+(threads-1), NULL, threadFunc, (void*)(intptr_t)newsock))) {
			perror2("pthread_create", err);
			exit(EXIT_FAILURE);
		}
    }
}

void shutdownContentServer() {
	int i, err;
	// wait all threads to be done
	for(i = 0; i < threads; i++) {
		printf("waiting thread no.%d\n", i);
		/* wait for thread termination */
		if((err = pthread_join(*(threadID+i), NULL))) {
			perror2("pthread_join", err);
			exit(EXIT_FAILURE);
		}
	}
	/* close sockets */
	close(sock);	// close server's socket
}