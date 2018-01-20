CC = gcc
CFLAGS = -Wall
CLIBS = -lpthread

MIRROR_INITIATOR_EXEC = MirrorInitiator
MIRROR_INITIATOR_OBJECTS = mirror_initiator/MirrorInitiator_main.o mirror_initiator/MirrorInitiator_functions.o
MIRROR_INITIATOR_SOURCES = mirror_initiator/MirrorInitiator_main.c mirror_initiator/MirrorInitiator_functions.c
MIRROR_INITIATOR_HEADERS = mirror_initiator/MirrorInitiator_header.h mirror_initiator/MirrorInitiator_functions.h

MIRROR_SERVER_EXEC = MirrorServer
MIRROR_SERVER_OBJECTS = mirror_server/MirrorServer_main.o mirror_server/MirrorServer_socket_functions.o mirror_server/MirrorServer_thread_functions.o
MIRROR_SERVER_SOURCES = mirror_server/MirrorServer_main.c mirror_server/MirrorServer_socket_functions.c mirror_server/MirrorServer_thread_functions.c
MIRROR_SERVER_HEADERS = mirror_server/MirrorServer_header.h mirror_server/MirrorServer_socket_functions.h mirror_server/MirrorServer_thread_functions.h

CONTENT_SERVER_EXEC = ContentServer
CONTENT_SERVER_OBJECTS = content_server/ContentServer_main.o content_server/ContentServer_socket_functions.o content_server/ContentServer_thread_functions.o content_server/ContentServer_commands.o
CONTENT_SERVER_SOURCES = content_server/ContentServer_main.c content_server/ContentServer_socket_functions.c content_server/ContentServer_thread_functions.c content_server/ContentServer_commands.c
CONTENT_SERVER_HEADERS = content_server/ContentServer_header.h content_server/ContentServer_socket_functions.h content_server/ContentServer_thread_functions.h content_server/ContentServer_commands.h

ALL_EXEC = $(MIRROR_INITIATOR_EXEC) $(MIRROR_SERVER_EXEC) $(CONTENT_SERVER_EXEC)
ALL_OBJECTS = $(MIRROR_INITIATOR_OBJECTS) $(MIRROR_SERVER_OBJECTS) $(CONTENT_SERVER_OBJECTS)
ALL_SOURCES = $(MIRROR_INITIATOR_OBJECTS) $(MIRROR_SERVER_SOURCES) $(CONTENT_SERVER_SOURCES)
ALL_HEADERS = $(MIRROR_INITIATOR_HEADERS) $(MIRROR_SERVER_HEADERS) $(CONTENT_SERVER_HEADERS)

executables: $(MIRROR_INITIATOR_OBJECTS) $(MIRROR_SERVER_OBJECTS) $(CONTENT_SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $(MIRROR_INITIATOR_EXEC) $(MIRROR_INITIATOR_OBJECTS)
	$(CC) $(CFLAGS) -o $(MIRROR_SERVER_EXEC) $(MIRROR_SERVER_OBJECTS) $(CLIBS)
	$(CC) $(CFLAGS) -o $(CONTENT_SERVER_EXEC) $(CONTENT_SERVER_OBJECTS) $(CLIBS)

MirrorInitiator_main.o: MirrorInitiator_main.c
	$(CC) $(CFLAGS) -c MirrorInitiator_main.c

MirrorInitiator_functions.o: MirrorInitiator_functions.c
	$(CC) $(CFLAGS) -c MirrorInitiator_functions.c

MirrorServer_main.o: MirrorServer_main.c
	$(CC) $(CFLAGS) -c MirrorServer_main.c

MirrorServer_socket_functions.o: MirrorServer_socket_functions.c
	$(CC) $(CFLAGS) -c MirrorServer_socket_functions.c

MirrorServer_thread_functions.o: MirrorServer_thread_functions.c
	$(CC) $(CFLAGS) -c MirrorServer_thread_functions.c

ContentServer_main.o: ContentServer_main.c
	$(CC) $(CFLAGS) -c ContentServer_main.c

ContentServer_socket_functions.o: ContentServer_socket_functions.c
	$(CC) $(CFLAGS) -c ContentServer_socket_functions.c

ContentServer_thread_functions.o: ContentServer_thread_functions.c
	$(CC) $(CFLAGS) -c ContentServer_thread_functions.c

ContentServer_commands.o: ContentServer_commands.c
	$(CC) $(CFLAGS) -c ContentServer_commands.c

.PHONY: clean

clean:
	rm -f $(ALL_EXEC) $(ALL_OBJECTS)

count:
	wc $(ALL_SOURCES) $(ALL_HEADERS)