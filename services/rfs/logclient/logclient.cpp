#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdio>

#define LOG_PORT 30301

int main(int argc, const char* argv[]) {
	if(argc != 2) {
		if(argc == 0)
			return EINVAL;

		printf("Usage: %s <IP>\n", argv[0]);
		return EINVAL;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(LOG_PORT);

	if(!inet_aton(argv[1], &addr.sin_addr)) {
		fprintf(stderr, "Invalid IP address %s!\n", argv[1]);
		return EINVAL;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("socket(AF_INET, SOCK_STREAM, 0)");
		return errno;
	}

	if(connect(sock, (const struct sockaddr*)&addr, sizeof(addr)) != 0) {
		perror("Could not connect");
		close(sock);
		return errno;
	}

	while(true) {
		unsigned char hdr[11];
		ssize_t r = recv(sock, hdr, 11, MSG_WAITALL);

		if(r < 0) {
			perror("recv");
			close(sock);
			return errno;
		} else if(r < 11) {
			printf("End of stream. Shutting down.\n");
			break;
		} else {
			unsigned char msg[1024];
			size_t msglen;
			unsigned char IO = hdr[0];
			uint64_t id;

			msglen  = (unsigned)(hdr[9]) << 8;
			msglen |= (unsigned)(hdr[10]);
			id = ((uint64_t)hdr[1]) << 56 | ((uint64_t)hdr[2]) << 48 |
				 ((uint64_t)hdr[3]) << 40 | ((uint64_t)hdr[4]) << 32 |
				 ((uint64_t)hdr[5]) << 24 | ((uint64_t)hdr[6]) << 16 |
				 ((uint64_t)hdr[7]) <<  8 | ((uint64_t)hdr[8]) <<  0;

			if((msglen < 32 && (msglen != 16 || IO != 'V')) || msglen > 1024) {
				printf("Invalid message length on log stream: %lu!\n", msglen);
				printf("Shutting down.\n");
				break;
			}

			r = recv(sock, msg, msglen, MSG_WAITALL);
			
			if(r < 0) {
				perror("recv");
				close(sock);
				return errno;
			} else if(((size_t)r) < msglen) {
				printf("End of stream reached. Shutting down.\n");
				break;
			}

			if(IO == 'I') {
				printf("Inbound message logged: \n");
			} else if(IO == 'O') {
				printf("Outbound message logged: \n");
			} else if(IO == 'V') {
				printf("IV message logged: \n");
			} else {
				printf("Ignoring invalid message on stream!\n");
				continue;
			}

			printf("\tID: %llx\n", (unsigned long long)id);
			printf("\tSize: %lu\n", msglen);
			printf("\tData: \n");
			printf("\t\t");

			for(size_t i = 0; i < msglen; i++) {
				if(i % 60 != 59) {
					printf("%02x ", (int)msg[i]);
				} else {
					printf("%02x\n\t\t", (int)msg[i]);
				}
			}

			printf("\n\n");
		}
	}

	close(sock);

	return 0;
}
