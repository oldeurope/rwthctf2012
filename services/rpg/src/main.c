#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include "game_engine.h"

static const unsigned int BI_POS = 0x09000000;
static const unsigned int NUM_PAGES = 20;

void sigchld_handler(int i) {
	wait(NULL);
}

int main(int argc, char** argv) {
//	printf("timeval is %u, cookie is %u\n",timeNow,cookie);
	// prevent enerving zombie processes
	struct sigaction sa;
	sa.sa_handler = &sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGCHLD, &sa, NULL);

    // prepare socket
    int servFd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if (servFd == -1) {
        printf("\x1B[37;1m[\x1B[31;1mError\x1B[37;1m]\x1B[0m Can't create socket.\n");
        exit(-1);
    }

    int optval = 1;
    setsockopt(servFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in sAddr;
    memset(&sAddr, 0, sizeof(sAddr));

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(8080);
    sAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(servFd,(struct sockaddr *)&sAddr, sizeof(sAddr)) == -1) {
        printf("\x1B[37;1m[\x1B[31;1mError\x1B[37;1m]\x1B[0m Can't bind to port.\n");
        exit(-1);
    }

    if (listen(servFd,5) == -1) {
        printf("\x1B[37;1m[\x1B[31;1mError\x1B[37;1m]\x1B[0m Socket won't listen.\n");
        exit(-1);
    }


    printf("\x1B[37;1m[\x1B[34mInfo\x1B[37;1m]\x1B[0m Accepting connections...\n");
    // accept loop
    while (1) {
        int cfd = accept(servFd,NULL,NULL);

		if (cfd < 0) // sigchld cancels the blocking call
			continue;

		if (fork() == 0) {
			printf("\x1B[37;1m[\x1B[35;1mDebug\x1B[37;1m]\x1B[0m New connection (%i)...\n",cfd);
			connectionMain(cfd);
		}

		close(cfd);

    }
	exit(0); // important otherwise the program crashs because of the faked stack cookie
}
