#include "MirrorServer_thread_functions.h"

/* condition variables */
pthread_cond_t allDone, cond_buffer_nonempty, cond_buffer_nonfull;

void createWorkerThreads() {
	if((workerIDs = malloc(threadnum*sizeof(pthread_t))) == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	srandom((unsigned int) time(NULL));	// initialize generator
	int i, err;
	for(i = 0; i < threadnum; i++){
		/* create a thread */
		if((err = pthread_create(workerIDs+i, NULL, workerThread, (void*)NULL))) {
			perror2("pthread_create", err);
			exit(EXIT_FAILURE);
		}
	}
}

void waitWorkerThreads() {
	int i, err;
	// wait until all workers are done
	printf("mirror server waiting workers\n");
	pthread_cond_wait(&allDone, &allDone_mtx);
	for(i = 0; i < threadnum; i++) {
		printf("waiting worker no.%d\n", i);
		/* wait for thread termination */
		if((err = pthread_join(*(workerIDs+i), NULL))) {
			perror2("pthread_join", err);
			exit(EXIT_FAILURE);
		}
	}
	printf("all %d threads have terminated\n", threadnum);
}

void createMirrorManagerThreads() {
	if((mirrorManagerIDs = malloc(numOfRequests*sizeof(pthread_t))) == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	srandom((unsigned int) time(NULL));	// initialize generator
	int i, err;
	for(i = 0; i < numOfRequests; i++){
		int index = i;
		/* create a thread */
		/* source: https://stackoverflow.com/questions/21323628/warning-cast-to-from-pointer-from-to-integer-of-different-size */
		if((err = pthread_create(mirrorManagerIDs+i, NULL, mirrorManagerThread, (void*)(intptr_t)index))) {
			perror2("pthread_create", err);
			exit(EXIT_FAILURE);
		}
	}
}

void waitMirrorManagerThreads() {
	int i, err;
	for(i = 0; i < numOfRequests; i++) {
		printf("waiting mirror manager no.%d\n", i);
		/* wait for thread termination */
		if((err = pthread_join(*(mirrorManagerIDs+i), NULL))) {
			perror2("pthread_join", err);
			exit(EXIT_FAILURE);
		}
	}
	printf("all %d threads have terminated\n", numOfRequests);
}

void *mirrorManagerThread(void *arg) {
	// lock buffer mutex
	pthread_mutex_lock(&buffer_mtx);
	while(worker || mirrorManager || buffer.count >= REQBUFFERSIZE)	{
		pthread_cond_wait(&buffer_write_cond, &buffer_mtx);
	}
	activeMirrorManagers++;
	mirrorManager = 1;
	// unlock buffer mutex
	pthread_mutex_unlock(&buffer_mtx);
	
	// preparation
	/* source: https://stackoverflow.com/questions/21323628/warning-cast-to-from-pointer-from-to-integer-of-different-size */
	int index = (intptr_t) arg;
	/* connect to content server */
	request[index].contentServerPtr = (struct sockaddr*)&(request[index].contentServer);
    /* create socket */
    if((request[index].contentServerSock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    /* find server address */
    /* gethostbyname called under mutex for thread-safety */
    // lock the mutex for getting host by name
    pthread_mutex_lock(&gethostbyname_mtx);
    if((request[index].contentServerHostEnt = gethostbyname(request[index].contentServerAddress)) == NULL) {
        perror("gethostbyname_r");
        exit(EXIT_FAILURE);
    }
    // unlock the mutex for getting host by name
    pthread_mutex_unlock(&gethostbyname_mtx);
    // note info about our content server
    request[index].contentServer.sin_family = AF_INET;    // Internet domain
    memcpy(&request[index].contentServer.sin_addr, request[index].contentServerHostEnt->h_addr, request[index].contentServerHostEnt->h_length);
    request[index].contentServer.sin_port = htons(request[index].contentServerPort);  // server port
    /* initiate connection */
    if(connect(request[index].contentServerSock, request[index].contentServerPtr, sizeof(request[index].contentServer)) < 0) {
        fprintf(stderr, "Thread %ld failed to connect to server's %s port %d\nThread %ld terminating...\n",
         		pthread_self(), request[index].contentServerAddress, request[index].contentServerPort, pthread_self());
    	
    	// lock buffer mutex
	    pthread_mutex_lock(&buffer_mtx);
    	numDevicesDone++;		// we found nothing, this counts as a served request
	    mirrorManager = 0;
	    activeMirrorManagers--;
	    pthread_cond_broadcast(&buffer_read_cond);
	    pthread_cond_signal(&buffer_write_cond);
	    printf("Mirror Manager %ld terminating...\n", pthread_self());
	    // unlock buffer mutex
	    pthread_mutex_unlock(&buffer_mtx);
    	
	    /* close connection */
    	close(request[index].contentServerSock);

    	/* exit thread */
		pthread_exit(NULL);
    }
    
    printf("Mirror Manager: Connecting to %s port %d\n", request[index].contentServerAddress, request[index].contentServerPort);
    /* send LIST command to content server */
    char buf[BUFSIZE];
	// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    memset(buf, '\0', BUFSIZE);
    // formulate our LIST command
    snprintf(buf, BUFSIZE, "LIST %d %d", request[index].id, request[index].delay);
	/* send LIST command to content server */
    while(write(request[index].contentServerSock, buf, BUFSIZE) < 0);

    /* receive answer */
    int matches = 0, found = 0;
    char **tempdirorfilename = NULL;
    
	// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    memset(buf, '\0', BUFSIZE);    

    /* source: https://stackoverflow.com/questions/35443876/executing-commands-via-sockets-with-popen */
    while(1) {
    	/* source: https://stackoverflow.com/questions/19127398/socket-programming-read-is-reading-all-of-my-writes */
    	int buflen;
    	while(read(request[index].contentServerSock, (char*)&buflen, sizeof(buflen)) < 0);
    	buflen = ntohl(buflen);

		/* source: https://stackoverflow.com/questions/22489981/linux-socket-client-server-program?rq=1 */
    	fflush(stdout);

    	while(read(request[index].contentServerSock, buf, buflen) < 0);
    	if(!strcmp(buf, "ok")) break;	// we are done

		/* source: https://stackoverflow.com/questions/22489981/linux-socket-client-server-program?rq=1 */
   	 	fflush(stdout);

    	char *temp1 = NULL, *temp2 = NULL, *tok1, *tok2;
    	// keep buffer's contents, the buffer ends in a "\n" and we don't want it for our comparisons
    	temp1 = malloc(strlen(buf)+1);
    	temp2 = malloc(strlen(buf)+1);
    	
    	strcpy(temp1, buf);
    	strcpy(temp2, buf);
    	tok1 = strtok(temp1, "/");
    	tok2 = strtok(temp2, "\n");	// take the "\n" character off our path
    	if(!strcmp(tok1, request[index].dirorfile) || !strcmp(tok2, request[index].dirorfile) || startsWith(request[index].dirorfile, buf)) {	// if we have eg a subfolder
    		matches++;			// one more match
    		tempdirorfilename = realloc(tempdirorfilename, matches*sizeof(char*));
    		tempdirorfilename[matches-1] = malloc(strlen(tok2)+1);
    		strcpy(tempdirorfilename[matches-1], tok2);
    	} else if(tempdirorfilename != NULL) break;	// we are done, because of the way the "find" command's results are structured
    	
		// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    	memset(buf, '\0', BUFSIZE);

		free(temp1);
    	free(temp2);
    }
    /* close connection */
    close(request[index].contentServerSock);

    /* act based on whether the answer is "affirmative" or "negatory" */
    if(tempdirorfilename != NULL) {	// in case there was an "affirmative" answer
    	found = 1;	// we found something
    	// update buffer's details
    	buffer.end = (buffer.end+1) % REQBUFFERSIZE;
    	buffer.count++;
    	// note our request's dir or file name
    	int i, end = buffer.end;
    	buffer.data[end].numOfContents = matches;
    	buffer.data[end].dirorfilename = malloc(matches*sizeof(char*));
    	for(i = 0; i < matches; i++) {
    		buffer.data[end].dirorfilename[i] = malloc(strlen(tempdirorfilename[i])+1);
    		strcpy(buffer.data[end].dirorfilename[i], tempdirorfilename[i]);
    		free(tempdirorfilename[i]);
    		tempdirorfilename[i] = NULL;
    	}
    	// note our request's ID
    	buffer.data[end].id = request[index].id;
    	// note our request's content server address
    	buffer.data[end].contentServerAddress = malloc(strlen(request[index].contentServerAddress)+1);
    	strcpy(buffer.data[end].contentServerAddress, request[index].contentServerAddress);
    	// note our request's content server port
    	buffer.data[end].contentServerPort = request[index].contentServerPort;
    } else {		// in case of "negatory" answer
    	printf("File %s not found in Content Server with address: %s and port: %d\n", request[index].dirorfile, request[index].contentServerAddress, request[index].contentServerPort);
    }
    /* free dynamically allocated memory */
    free(tempdirorfilename);
    tempdirorfilename = NULL;

    // lock buffer mutex
    pthread_mutex_lock(&buffer_mtx);
    mirrorManager = 0;
    activeMirrorManagers--;
    if(!found)				// if we found nothing, count this as a served request
    	numDevicesDone++;
	pthread_cond_broadcast(&buffer_read_cond);
    pthread_cond_signal(&buffer_write_cond);
    // unlock buffer mutex
    pthread_mutex_unlock(&buffer_mtx);

    /* exit thread */
	pthread_exit(NULL);
}

void *workerThread(void *arg) {
	int over = 0;

	while(!over) {
		// lock buffer mutex
		pthread_mutex_lock(&buffer_mtx);
		while((worker || mirrorManager || buffer.count <= 0) && !done && (numDevicesDone < numOfRequests || numDevicesDone == 0)) {
			pthread_cond_wait(&buffer_read_cond, &buffer_mtx);
		}
		if(done){
			if(!activeMirrorManagers && buffer.count <= 0 && numDevicesDone == numOfRequests) {
				over = 1;	// worker thread should terminate
				// if this is the last worker, then broadcast all done
				if(!activeWorkers) {
					pthread_cond_signal(&allDone);
				}
			}
			// unlock buffer mutex
			pthread_mutex_unlock(&buffer_mtx);
			break;
		}
		activeWorkers++;
		worker = 1;
		// unlock buffer mutex
		pthread_mutex_unlock(&buffer_mtx);

		/* worker thread fetches a file from content server */
		workerFetch();

		// lock buffer mutex
		pthread_mutex_lock(&buffer_mtx);
		activeWorkers--;
		worker = 0;
		if(!activeMirrorManagers && buffer.count <= 0 && numDevicesDone == numOfRequests) {
			over = 1;	// worker thread should terminate
			// if this is the last worker, then broadcast all done
			if(!activeWorkers) {	// if this was the last active worker
				done = 1;	// we are done
				pthread_cond_signal(&allDone);
			}
		}
		// signal to a mirror manager
		pthread_cond_signal(&buffer_write_cond);
		// broadcast to all workers that may be waiting
		pthread_cond_broadcast(&buffer_read_cond);
		// unlock buffer mutex
		pthread_mutex_unlock(&buffer_mtx);
	}

	printf("Worker %ld terminating...\n", pthread_self());
    printf("%d devices done at this point\n", numDevicesDone);
    
    /* exit thread */
	pthread_exit(NULL);
}

void workerFetch() {
	// print read item's details
	int i, start = buffer.start;

	if(buffer.count == 0) return;	// don't try to fetch anyting if the buffer is empty

	printf("%d, %s, %d, %s, %d\n", buffer.data[start].id, buffer.data[start].dirorfilename[0], buffer.data[start].numOfContents, buffer.data[start].contentServerAddress, buffer.data[start].contentServerPort);
	for(i = 0; i < buffer.data[start].numOfContents; i++) {
		/* if this is a file, request it from server */
		if(!isDirectory(buffer.data[start].dirorfilename[i])) {
			/* source: https://codereview.stackexchange.com/questions/43914/client-server-implementation-in-c-sending-data-files */
			// printf("NOT A DIRECTORY\n");
			// setup the path
		    char path[512], port[20];
		    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
		    memset(port, '\0', sizeof(port));
		    // port to string
		    sprintf(port, "%d", buffer.data[start].contentServerPort);
			// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
		    memset(path, '\0', sizeof(path));
		    // create path's name
		    strcpy(path, dirname);
		    strcat(path, "/");
		    strcat(path, buffer.data[start].contentServerAddress);
		    strcat(path, "_");
		    strcat(path, port);
		    strcat(path, "/");
		    strcat(path, buffer.data[start].dirorfilename[i]);
		    // printf("path = %s\n", path);
		    /* make path's directories or subdirectories if needed */
			my_mkdir(path, buffer.data[start].contentServerPort, 0777);
		    /* create socket */
		    int contSock = 0;
		    if((contSock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		        perror("socket");
		        exit(EXIT_FAILURE);
		    }
		    /* find server address */
		    hostent* contHostEnt;
		    /* gethostbyname called under mutex for thread-safety */
		    // lock the mutex for getting host by name
		    pthread_mutex_lock(&gethostbyname_mtx);
		    if((contHostEnt = gethostbyname(buffer.data[start].contentServerAddress)) == NULL) {
		        perror("gethostbyname");
		        exit(EXIT_FAILURE);
		    }
		    // unlock the mutex for getting host by name
		    pthread_mutex_unlock(&gethostbyname_mtx);
		    /* initialize sockaddr_in data structure */
		    sockaddr_in contAddr;
		    contAddr.sin_family = AF_INET;	// internet domain
		    contAddr.sin_port = htons(buffer.data[start].contentServerPort);	// server port
		    memcpy(&contAddr.sin_addr, contHostEnt->h_addr, contHostEnt->h_length);
		    /* initiate connection */
		    if(connect(contSock, (sockaddr*)&contAddr, sizeof(contAddr)) < 0) {
		        fprintf(stderr, "Thread %ld failed to connect to server's %s port %d\nThread %ld terminating...\n",
		         		pthread_self(), buffer.data[start].contentServerAddress, buffer.data[start].contentServerPort, pthread_self());
		    	/* close connection */
		        close(contSock);
		    	/* exit thread */
				pthread_exit(NULL);
		    }
		    printf("\tWorker: Connecting to %s port %d\n", buffer.data[start].contentServerAddress, buffer.data[start].contentServerPort);
		    /* send FETCH command to content server */
		    char buf[BUFSIZE];
			// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
		    memset(buf, '\0', BUFSIZE);
		    // formulate our FETCH command
		    if(snprintf(buf, BUFSIZE, "FETCH %d %s", buffer.data[start].id, buffer.data[start].dirorfilename[i]) < 0) {
		    	perror("snprintf");
		    	exit(EXIT_FAILURE);
		    }
			/* send FETCH command to content server */
		    while(write(contSock, buf, BUFSIZE) < 0);
		    printf("\tWorker %ld waiting for file\n", pthread_self());
		    /* create file where data will be stored */
		    // open the file -- append-binary mode, create it if necessary
		    FILE *fp = fopen(path, "ab");
		    if(fp == NULL) {
		    	perror("fopen");
		    	exit(EXIT_FAILURE);
		    }

		    /* receive data in chuncks of BUFSIZE bytes */
		    int bytesReceived = 0, fileSize = 0;

		    while((bytesReceived = read(contSock, buf, BUFSIZE)) > 0 && strcmp(buf, "done") != 0 && strcmp(buf, "empty-file") != 0) {
		    	bytesTransfered += bytesReceived;
		    	fileSize += bytesReceived;
		    	printf("Bytes received %d\n", bytesReceived);
		    	fwrite(buf, 1, bytesReceived, fp);
			    // send confirmation that data-chunk was received
			    strcpy(buf, "ok");
			    while(write(contSock, buf, BUFSIZE) < 0);
		    }
		    printf("transfer done\n");
		    filesTransfered++;
		    values = realloc(values, filesTransfered*sizeof(double));
		    values[filesTransfered-1] = fileSize;
		    if(fileSize > maxSize) maxSize = fileSize;
		    if(fileSize < minSize) minSize = fileSize;

		    if(bytesReceived < 0)
		    	perror("Read error");

		    fclose(fp);
		    /* close connection */
		    close(contSock);
		}
		/* else if this is a directory, create it */
		else {
			// printf("IS A DIRECTORY\n");
			/* create the directory, if needed */
		    // setup the path
		    char path[512], port[20];
		    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
		    memset(port, '\0', sizeof(port));
		    sprintf(port, "%d", buffer.data[start].contentServerPort);
			// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
		    memset(path, '\0', sizeof(path));
		    // create path's name
		    strcpy(path, dirname);
		    strcat(path, "/");
		    strcat(path, buffer.data[start].contentServerAddress);
		    strcat(path, "_");
		    strcat(path, port);
		    strcat(path, "/");
		    strcat(path, buffer.data[start].dirorfilename[i]);
		    // printf("path = %s\n", path);
		    /* check if directory exist */
		    /* source: https://stackoverflow.com/questions/28247719/create-a-file-and-its-parents-directories-in-c */
			Stat status = {0};
			if(stat(path, &status) == -1) {
			  /* create path's directory */
			  mkdir(path, 0777);
			}
		}
		free(buffer.data[start].dirorfilename[i]);
		buffer.data[start].dirorfilename[i] = NULL;
	}

	free(buffer.data[start].dirorfilename);
	buffer.data[start].dirorfilename = NULL;

	free(buffer.data[start].contentServerAddress);
	buffer.data[start].contentServerAddress = NULL;

	// update buffer's details
	buffer.start = (buffer.start+1) % REQBUFFERSIZE;
	buffer.count--;

	// we have one more served request
	numDevicesDone++;
}

int isDirectory(const char *path) {
   Stat statbuf = {0};
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

/* source: https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix */
void my_mkdir(const char *dir, int portnum, mode_t mode) {
    char tmp[512];

    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
	memset(tmp, '\0', sizeof(tmp));

    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);

    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++)
        if(*p == '/') {
            *p = 0;
            mkdir(tmp, mode);
            *p = '/';
    }
    if(isDirectory(tmp)) mkdir(tmp, mode);
}


/* source: https://stackoverflow.com/questions/4770985/how-to-check-if-a-string-starts-with-another-string-in-c */
int startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}
