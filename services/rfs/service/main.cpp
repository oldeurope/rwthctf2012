#include <openssl/evp.h>
#include <openssl/engine.h>
#include <stdexcept>
#include <exception>
#include "id-base.h"
#include "globals.h"
#include "net.h"
#include "client.h"
#include <csignal>
#include <openssl/ssl.h>

volatile int stop = 0;
using namespace std;

void stop_handler(int sig) {
	stop = 1;
}

int main() {
	signal(SIGINT, stop_handler);
	signal(SIGTERM, stop_handler);

	SSL_load_error_strings();
	SSL_library_init();
	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();

	IDBase base;
	
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0) {
		perror("socket");
		throw runtime_error("Could not create listener socket!");
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(MAIN_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(listener, (const sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(listener);
		throw runtime_error("Could not bind main socket!");
	}

	if(listen(listener, SOMAXCONN) < 0) {
		perror("listen");
		close(listener);
		throw runtime_error("Could not listen on main socket!");
	}

	set_nonblocking(listener);

	while(!stop) {
		fd_set rfds;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 250000;
		FD_ZERO(&rfds);
		FD_SET(listener, &rfds);

		if(select(listener+1, &rfds, 0, 0, &tv) > 0) {
			int sock = accept(listener, 0, 0);
			if(sock >= 0) {
				new Client(base, sock);
			} else {
				perror("accept");
			}
		}
	}

	close(listener);

	return 0;
}
