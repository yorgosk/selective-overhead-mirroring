#include "ContentServer_thread_functions.h"

void* threadFunc(void* arg) {
	int newsock = (intptr_t) arg;
	/* listen command */
	listenCommand(newsock);
	/* close connection */
	shutdown(newsock, 1); // Further sends are disallowed
	/* exit thread */
	pthread_exit(NULL);
}

void listenCommand(int newsock) {
	char buf[BUFSIZE];
	/* receive command */
	while(read(newsock, buf, BUFSIZE) < 0);
	/* listen command from mirror server */
	processCommand(buf, newsock);
}

void processCommand(char* command, int newsock) {
	char buf[BUFSIZE];
	// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
   	memset(buf, '\0', BUFSIZE);
	char* temp = NULL;	// I will work with a temporary string, so that I don't lose the initial command while tokenizing
	temp = malloc(strlen(command)+1);
	strcpy(temp, command);

	char* op;
	op = strtok(temp, " ");	// take the commands "operation" keyword
	if(!strcmp(op, "LIST")) {
		list(command, newsock);
	} else if(!strcmp(op, "FETCH")) {
		fetch(command, newsock);
	} else {
		perror("invalid command");
	}
	// free dynamically allocated memory
	free(temp);
}

void sanitize(char *str) {
	char *src, *dest;
	for(src = dest = str; *src; src++)
		if (*src == '/' || isalnum (*src))
			*dest++ = *src;
	*dest = '\0';
}
