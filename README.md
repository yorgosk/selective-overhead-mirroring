# Project: Project3 - Selective Overhead Mirroring System

Project Developer: Georgios Kamaras - sdi1400058

Course: K24 Systems Programming, Spring 2017

Date: 31/05/2017

Development Platform:
*	GNU/Linux Ubuntu 16.04
*	gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4)

## Included Files
*	makefile
*	MirrorInitiator_header.h (mirror initiator's main library-file, links all of the
	mirror initiator program's modules together)
*	MirrorInitiator_main.c (mirror initiator's main function)
*	MirrorInitiator_functions.h MirrorInitiator_functions.c (various functions, utilized throughout
	mirror initiator's program)
*	MirrorServer_header.h (mirror server's main library-file, links all of the
	mirror server program's modules together)
*	MirrorServer_main.c (mirror server's main function)
*	MirrorServer_socket_functions.h MirrorServer_socket_functions.c (various socket related functions,
	utilized throughout mirror server's program)
*	MirrorServer_thread_functions.h MirrorServer_thread_functions.c (various thread related functions,
	utilized throughout mirror server's program)
*	ContentServer_header.h (content server's main library-file, links all of the
	content server program's modules together)
*	ContentServer_main.c (content server's main function)
*	ContentServer_socket_functions.h ContentServer_socket_functions.c (various socket related functions,
	utilized throughout content server's program)
*	ContentServer_thread_functions.h ContentServer_thread_functions.c (various thread related functions,
	utilized throughout content server's program)
*	(and README)

## Compilation
Use ```make``` command

## Cleaning
Use ```make clean``` command

## Usage
*	(1) first, run the content server(s) and the mirror server using the commands: ```./ContentServer -p <port> -d <dirorfilename>``` and ```./MirrorServer -p <port> -m <dirname> -w <threadnum>```,
	e.g. ```./ContentServer -p 15100 -d test-dir1``` and ```./MirrorServer -p 15000 -m mydir -w 5```
*	(2) then, run the mirror initiator using the command: ```./MirrorInitiator -n <MirrorServerAddress> -p <MirrorServerPort> -s <ContentServerAddress1:ContentServerPort1:dirorfile1:delay1, ContentServerAddress2:ContentServerPort2:dirorfile2:delay2, ...>```,
	e.g. ```./MirrorInitiator -n localhost -p 15000 -s localhost:15100:test-dir1/libraries/:1 localhost:15200:test-dir2:2```

## Technical Details
*	For my implementation I followed all of the exercise's specifications both in paper and on Piazza.
	My goal was to develop a Selective Overhead Mirroring System able to manage a large amount of
	mirroring requests under various usage conditions.
*	My Selective Overhead Mirroring System consists of three main programs: the Mirror Initiator's program,
	the Mirror Server's program and the Content Server's program. Ideally and rationally, in order for the
	system to operate successfully, it is required that at least one Content Server is active, exactly one
	Mirror Server and exactly one Mirror Initiator. It's also required that the Content Server and the Mirror
	Server are active (up and running) before the Mirror Initiator's activation and, of course it's neccessary
	that the user passes the right parameters to each program, so they can connect successfully with each
	other.
*	The main idea is that the user, through the Mirror Initiator, requests certain content from certain
	(Content) servers. The user's requests are transfered to the Mirror Server, which processes them and
	makes sure to spot the content in the Content Server(s) and fetch it. The Mirror Server stores the fetched
	content locally and sends any statistical details that he noted from the whole process to the Mirror Initiator.
	After all these are done, the Mirror Server and the Mirror Initiator have terminated and the Content Servers
	are still running and waiting to server the next request(s).
*	The project has been tested successfully (0 ERRORS) on the Valgrind memory-checking environment.

## Appendix I - External Sources
*	For "relative dispersion" statistics I consulted the link: http://sciencing.com/calculate-dispersion-10018216.html
*	For checking if a dir-or-file name is a directory or a file, I consulted the link:
	https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
*	To use the "mkdir" system call recursively on unix, I consulted the link:
	https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
*	To check for string prefixes, I consulted the link:
	https://stackoverflow.com/questions/4770985/how-to-check-if-a-string-starts-with-another-string-in-c
*	To create a file and it's parent directories, I consulted the link:
	https://stackoverflow.com/questions/28247719/create-a-file-and-its-parents-directories-in-c
*	For checking if a file is empty, I consulted the link:
	https://stackoverflow.com/questions/30133210/check-if-file-is-empty-or-not-in-c
*	For guidance in implementing the file transfer between client and server, I consulted the link:
	https://codereview.stackexchange.com/questions/43914/client-server-implementation-in-c-sending-data-files
*	To execute commands via sockets with "popen", I consulted the link:
	https://stackoverflow.com/questions/35443876/executing-commands-via-sockets-with-popen
*	I was inspired to send the size of the info, before the info itself, in order to fix a serious bug by this
	link: https://stackoverflow.com/questions/19127398/socket-programming-read-is-reading-all-of-my-writes
*	To make the binded sockets immediately re-usable after being closed, I consulted the link:
	https://stackoverflow.com/questions/22489981/linux-socket-client-server-program?rq=1
*	Furthermore, various code segments, utility functions and programming techniques that I have used, have been
	taken or inspired from Prof. Alexis Delis "System Programming" (K24) course slides,
	Department of Informatics and Telecommunications, NKUA, Spring 2017.

## Contact - feedback
Georgios Kamaras: sdi1400058@di.uoa.gr
