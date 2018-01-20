#include "ContentServer_commands.h"

void list(char* command, int newsock) {
	char buf[BUFSIZE];
	// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
   	memset(buf, '\0', BUFSIZE);

	char *tok, *temp = NULL;	// work on a temporary variable, so that the original comman is not lost while tokenizing
	int contentServerID, delay;

	temp = malloc(strlen(command)+1);
	strcpy(temp, command);

	tok = strtok(temp, " ");	// take "LIST" away
	tok = strtok(NULL, " ");	// take content server ID
	contentServerID = atoi(tok);
	tok = strtok(NULL, " ");	// take delay
	delay = atoi(tok);

	numOfRequests++;
	reqInfo = realloc(reqInfo, numOfRequests*sizeof(request));
	reqInfo[numOfRequests-1].contentServerID = contentServerID;
	reqInfo[numOfRequests-1].delay = delay;

	/* run a "find" command (with no parameters) to analytically list all dirs and files */
	// formulate find command
	char* comm = NULL;
	comm = malloc(strlen("find ")+strlen(dirorfilename)+1);
	strcpy(comm, "find ");
	strcat(comm, dirorfilename);
	// printf("comm: %s\n", comm);	// for debugging
	/* source: https://stackoverflow.com/questions/35443876/executing-commands-via-sockets-with-popen */
	FILE *fp = popen(comm, "r");
	if(fp == NULL) {
      	perror("popen");
      	exit(EXIT_FAILURE);
   	}

   	// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
   	memset(buf, '\0', BUFSIZE);
   	while(fgets(buf, BUFSIZE, fp) != NULL) {
   		// printf("fgets buf = \"%s\"\n", buf);
   		/* source: https://stackoverflow.com/questions/19127398/socket-programming-read-is-reading-all-of-my-writes */
   		int datalen = strlen(buf);	// number of bytes in data
   		int tmp = htonl(datalen);
   		while(write(newsock, (char*)&tmp, sizeof(tmp)) < 0);
   		while(write(newsock, buf, datalen) < 0);
	}

	/* source: https://stackoverflow.com/questions/22489981/linux-socket-client-server-program?rq=1 */
    fflush(stdout);
    
	// send code that we are done
	int datalen = strlen("ok")+1;	// number of bytes in data
   	int tmp = htonl(datalen);
   	while(write(newsock, (char*)&tmp, sizeof(tmp)) < 0);
   	strcpy(buf, "ok");
   	while(write(newsock, buf, datalen) < 0);
	// close file
	pclose(fp);
	// free dynamically allocated memory
	free(comm);
	free(temp);
}

void fetch(char* command, int newsock) {
	char buf[BUFSIZE];
	/* send "fetching" to mirror server */
	// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
   	memset(buf, '\0', BUFSIZE);

	char *tok, *dirorfile = NULL, *temp = NULL;	// work on a temporary variable, so that the original comman is not lost while tokenizing
	int i, contentServerID, delay;

	temp = malloc(strlen(command)+1);
	strcpy(temp, command);

	tok = strtok(temp, " ");	// take "FETCH" away
	tok = strtok(NULL, " ");	// take content server ID
	contentServerID = atoi(tok);
	tok = strtok(NULL, " ");	// take dirorfilename
	dirorfile = malloc(strlen(tok)+1);
	strcpy(dirorfile, tok);

	// find delay
	// printf("fetch: numOfRequests = %d\n", numOfRequests);
	for(i = 0; i < numOfRequests; i++) {
		// printf("id no.%d = %d\n", i, reqInfo[i].contentServerID);
		if(reqInfo[i].contentServerID == contentServerID) {
			delay = reqInfo[i].delay;
			break;	// our search is done
		}
	}

	// printf("Delaying for %ds\n", delay);
	sleep(delay);
	// printf("Serving dirorfile: %s with delay %d\n", dirorfile, delay);

	/* basically, we know that we are going to transfer a file, because we ask the content server 
	about only what the mirror server is missing from the whole folder structure, if there is one */
	// check if the file is empty
	/* source: https://stackoverflow.com/questions/30133210/check-if-file-is-empty-or-not-in-c */
	struct stat fileStat;
	stat(dirorfile, &fileStat);
	if(fileStat.st_size != 0) {	// if file is not empty, then send it
		/* source: https://codereview.stackexchange.com/questions/43914/client-server-implementation-in-c-sending-data-files */
		/* open the file we wish to transfer */
		FILE *fp = fopen(dirorfile, "rb");
		if(fp == NULL) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}
		// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
	   	memset(buf, '\0', BUFSIZE);
		/* read data from file and send it */
		while(1) {
			/* read file in chunks of BUFSIZE bytes */
	   		int nread = fread(buf, 1, BUFSIZE, fp);
	   		printf("Bytes read %d\n", nread);
	   		/* if fread was successful, then send data */
	   		if(nread > 0) {
	   			printf("Sending data\n");
	   			while(write(newsock, buf, nread) < 0);
	   		}
	   		// wait for confirmation that the chunk was received
	   		while(read(newsock, buf, BUFSIZE) < 0 || strcmp(buf, "ok") != 0);
	   		if(nread < BUFSIZE) {
	   			if(feof(fp)) printf("End of file\n");
	   			if(ferror(fp)) perror("Error reading");
	   			break;
	   		}
			// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
	   		memset(buf, '\0', BUFSIZE);
		}
		// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
	   	memset(buf, '\0', BUFSIZE);
		// notify the mirror server that file transfer is over
	   	strcpy(buf, "done");
	   	while(write(newsock, buf, BUFSIZE) < 0);
		printf("Served\n");
		fclose(fp);
	} else {	// else inform mirror server that the file he requested is empty, no need to send it, he can just create it locally for himself
		char buf[BUFSIZE];
		// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
	   	memset(buf, '\0', BUFSIZE);
	   	strcpy(buf, "empty-file");
		while(write(newsock, buf, BUFSIZE) < 0);
	}

	free(dirorfile);
	free(temp);
}