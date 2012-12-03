#include <stdint.h>
#include <string>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include <csignal>
#include <cerrno>
#include <ctime>
#include <sys/time.h>
#include <pthread.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>
#include <openssl/ssl.h>

#include "getkey.h"

// How many ms to wait for connect/send/recv
#define CONNECT_TIMEOUT 25000
#define SEND_TIMEOUT 5000
#define RECV_TIMEOUT 5000
#define WAIT_LOGGED 10000
#define GLOBAL_TIMEOUT 35000
#define LOG_PORT 30301
#define MAIN_PORT 30300

bool time_lt(const struct timeval& t1, const struct timeval& t2) {
	return t1.tv_sec < t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

void time_sub(struct timeval& result, const struct timeval& t1, const struct timeval& t2) {
	result = t1;
	
	if(t1.tv_usec < t2.tv_usec) {
		result.tv_sec -= 1;
		result.tv_usec += 1000000;
	}

	result.tv_sec -= t2.tv_sec;
	result.tv_usec -= t2.tv_usec;
}

void fillAddr(const std::string& ip, struct sockaddr_in& addr, uint16_t port) {
	std::memset(&addr, 0, sizeof(addr));

	if(!inet_aton(ip.c_str(), &addr.sin_addr)) {
		fprintf(stderr, "Invalid IP given: %s!\n", ip.c_str());
		exit(3);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
}

void* connectTo(void* _addr) {
	const sockaddr* addr = (const sockaddr*)_addr;
	int s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	int r;
	socklen_t optsize = sizeof(int);

	if(s < 0) {
		perror("connectTo failed: socket");
		exit(3);
	}

	if((r = connect(s, addr, sizeof(struct sockaddr_in))) != 0 && errno != EINPROGRESS) {
		fprintf(stdout, "Could not connect: %s!\n", strerror(errno));
		exit(1);
	}

	if(r != 0) {
		fd_set wfds, rfds, efds;
		FD_ZERO(&wfds);
		FD_ZERO(&rfds);
		FD_ZERO(&efds);
		FD_SET(s, &wfds);
		FD_SET(s, &rfds);
		FD_SET(s, &efds);

		struct timeval cur, tv, deadline;
		tv.tv_sec = CONNECT_TIMEOUT / 1000;
		tv.tv_usec = (CONNECT_TIMEOUT % 1000) * 1000;
		
		gettimeofday(&deadline, 0);
		deadline.tv_sec += CONNECT_TIMEOUT / 1000;
		deadline.tv_usec = (CONNECT_TIMEOUT % 1000) * 1000;

		while((r = select(s+1, &rfds, &wfds, &efds, &tv)) == 0) {
			gettimeofday(&cur, 0);
			if(!time_lt(cur, deadline)) {
				fprintf(stdout, "Could not connect on port %u: Timeout!\n", (unsigned)ntohs(((const struct sockaddr_in*)addr)->sin_port));
				exit(1);
			} else {
				fprintf(stderr, "Spurious select wakeup.\n");
				time_sub(tv, deadline, cur);
				tv.tv_sec += 1;
			}
		}

		if(r < 0) {
			if(errno != EBADF) {
				perror("internal error: select");
				exit(3);
			}
		}

		if(getsockopt(s, SOL_SOCKET, SO_ERROR, &r, &optsize) < 0) {
			perror("getsockopt");
			exit(3);
		}

		if(r != 0) {
			fprintf(stdout, "Could not connect: %s!\n", strerror(r));
			exit(1);
		}
	}

	return (void*)(uintptr_t)s;
}

int sock_log, sock_main;

void setupConnection(const std::string& ip) {
	struct sockaddr_in addrMain, addrLog;
	pthread_t lt, mt;

	fillAddr(ip, addrMain, MAIN_PORT);
	fillAddr(ip, addrLog, LOG_PORT);

	if(
		pthread_create(&lt, 0, connectTo, &addrLog) != 0 || 
		pthread_create(&mt, 0, connectTo, &addrMain) != 0
	) {
		perror("pthread_create");
		exit(3);
	}

	void* ps1, *ps2;
	pthread_join(lt, &ps1);
	pthread_join(mt, &ps2);

	sock_log  = (int)(uintptr_t)ps1;
	sock_main = (int)(uintptr_t)ps2;
}

typedef std::vector<unsigned char> buf;
std::vector<buf> loggedMessages;
std::vector<buf> expectedMessages;
volatile int mainDone = 0;
pthread_t loggerThread;

void doRcvLog(unsigned char* curBuf, size_t& curRecv) {
	size_t curMsgSize;

	if(curRecv < 11) {
		ssize_t r = recv(sock_log, &curBuf[curRecv], 11-curRecv, 0);

		if(r == 0) {
			fprintf(stdout, "Error - unexpected end of logging stream!\n");
			exit(2);
		} else if(r <= 0) {
			if(errno != EAGAIN && errno != EWOULDBLOCK) {
				fprintf(stdout, "Error on logging stream: %s!\n", strerror(errno));
				exit(2);
			} else {
				return;
			}
		} else {
			curRecv += r;
		}
	} 

	if(curRecv >= 11) {
		curMsgSize  = ((unsigned)curBuf[9]) << 8;
		curMsgSize += ((unsigned)curBuf[10]);

		if((curMsgSize < 32 && (curMsgSize != 16 || curBuf[0] != 'V')) || curMsgSize > 1024) {
			fprintf(stdout, "Error - invalid message on logging stream - size %lu!\n", curMsgSize);
			exit(2);
		}

		if(curRecv < 11 + curMsgSize) {
			ssize_t r = recv(sock_log, &curBuf[curRecv], 11 + curMsgSize - curRecv, 0);

			if(r == 0) {
				fprintf(stdout, "Error - unexpected end of logging stream!\n");
				exit(2);
			} else if(r <= 0) {
				if(errno != EAGAIN && errno != EWOULDBLOCK) {
					fprintf(stdout, "Error on logging stream: %s!\n", strerror(errno));
					exit(2);
				} else {
					return;
				}
			} else {
				curRecv += r;
			}
		}

		if(curRecv == 11+curMsgSize) {
			loggedMessages.push_back(buf(curBuf, curBuf+curRecv));
			curRecv = 0;
		}
	}
}

void time_add(struct timeval &t, time_t t1, suseconds_t t2) {
	t.tv_sec += t1;
	t.tv_usec += t2;

	if(t.tv_usec > 1000000) {
		t.tv_usec -= 1000000;
		t.tv_sec += 1;
	}
}

void* loggerMain(void*) {
	int sfd;
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &mask, 0);

	sfd = signalfd(-1, &mask, SFD_NONBLOCK);
	if(sfd < 0) {
		perror("signalfd");
		exit(3);
	}
	
	size_t curRecv = 0;
	unsigned char curBuf[1024 + 43];

	while(!mainDone) {
		fd_set rfds;
		
		FD_ZERO(&rfds);
		FD_SET(sfd, &rfds);
		FD_SET(sock_log, &rfds);

		int s = select(sfd > sock_log ? sfd+1 : sock_log+1, &rfds, 0, 0, 0);
		if(s < 0) {
			fprintf(stdout, "Error - logger socket broken!\n");
			exit(2);
		}

		doRcvLog(curBuf, curRecv);
	}

	close(sfd);

	size_t i = 0, j = 0;
	for(; i < expectedMessages.size() && j < loggedMessages.size(); j++) {
		if(expectedMessages[i] == loggedMessages[j]) {
			++i;
		}
	}

	struct timeval timeout, curtime;
	gettimeofday(&timeout, 0);
	time_add(timeout, WAIT_LOGGED / 1000, (WAIT_LOGGED % 1000) * 1000);
	
	while(i < expectedMessages.size() && (gettimeofday(&curtime, 0), time_lt(curtime, timeout))) {
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(sock_log, &rfds);

		struct timeval rem;
		time_sub(rem, timeout, curtime);

		int s = select(sock_log + 1, &rfds, 0, 0, &rem);
		if(s < 0) {
			fprintf(stdout, "Error - logger socket broken!\n");
			exit(2);
		} else if(s > 0) {
			size_t expSize = loggedMessages.size();
			doRcvLog(curBuf, curRecv);

			if(expSize < loggedMessages.size()) {
				if(loggedMessages[expSize] == expectedMessages[i]) {
					++i;
				}
			}
		}
	}

	if(expectedMessages.size() == i) {
		return 0;
	} else {
		if(!(gettimeofday(&curtime, 0), time_lt(curtime, timeout))) {
			fprintf(stdout, "Error - logging broken: Timeout while trying to find expected log messages!\n");
			exit(2);
		}

		fprintf(stdout, "Error - logging does not work correctly: Failed to find expected messages!\n");
		exit(2);
	}
}

void startLoggerThread() {
	if(pthread_create(&loggerThread, 0, loggerMain, 0) != 0) {
		perror("pthread_create");
		exit(3);
	}
}

void setMainDone() {
	mainDone = 1;
	pthread_kill(loggerThread, SIGUSR1);
}

void do_send(const unsigned char* buffer, size_t length) {
	struct timeval timeout, curtime;
	fd_set wfds;
	size_t sent = 0;

	gettimeofday(&timeout, 0);
	time_add(timeout, SEND_TIMEOUT / 1000, (SEND_TIMEOUT % 1000) * 1000);

	while(sent < length && (gettimeofday(&curtime, 0), time_lt(curtime, timeout))) {
		struct timeval tv;
		time_sub(tv, timeout, curtime);

		FD_ZERO(&wfds);
		FD_SET(sock_main, &wfds);
		
		int s = select(sock_main + 1, 0, &wfds, 0, &tv);
		if(s <= 0) {
			fprintf(stdout, "Timeout while sending!\n");
			exit(2);
		} else {
			s = send(sock_main, &buffer[sent], length-sent, MSG_NOSIGNAL);

			if(s < 0) {
				fprintf(stdout, "Could not send: %s!\n", strerror(errno));
				exit(2);
			} else {
				sent += s;
			}
		}
	}
}

void do_recv(unsigned char* buffer, size_t length) {
	struct timeval timeout, curtime;
	fd_set rfds;
	size_t recvd = 0;

	gettimeofday(&timeout, 0);
	time_add(timeout, RECV_TIMEOUT / 1000, (RECV_TIMEOUT % 1000) * 1000);

	while(recvd < length && (gettimeofday(&curtime, 0), time_lt(curtime, timeout))) {
		struct timeval tv;
		time_sub(tv, timeout, curtime);

		FD_ZERO(&rfds);
		FD_SET(sock_main, &rfds);

		int s = select(sock_main + 1, &rfds, 0, 0, &tv);
		if(s <= 0) {
			fprintf(stdout, "Timeout while receiving!\n");
			exit(2);
		} else {
			s = recv(sock_main, &buffer[recvd], length - recvd, 0);

			if(s < 0) {
				fprintf(stdout, "Could not receive: %s!\n", strerror(errno));
				exit(2);
			} else if(s == 0) {
				fprintf(stdout, "Could not receive: Unexpected end of stream!\n");
				exit(2);
			} else {
				recvd += s;
			}
		}
	}
}

typedef struct aes_ctr_state {
	AES_KEY key;
	uint8_t ctr[16], ecount_buf[16];
	unsigned num;
} cipher_state;

static void encrypt_msg(unsigned char *out, cipher_state* cipher, const unsigned char* key, const unsigned char* msg, size_t inlen, size_t* outlen) {
	size_t length = inlen + 32;
	out[0] = (unsigned char)(length >> 8);
	out[1] = (unsigned char)(length);

	AES_ctr128_encrypt(msg, out+2, inlen, &cipher->key, cipher->ctr, cipher->ecount_buf, &cipher->num);

	HMAC_CTX hmac;
	HMAC_CTX_init(&hmac);
	HMAC_Init(&hmac, key, 32, EVP_sha256());
	HMAC_Update(&hmac, out+2, inlen);
	HMAC_Final(&hmac, out+inlen+2, 0);
	HMAC_CTX_cleanup(&hmac);

	*outlen = length+2;
}

void do_recv_msg(unsigned char* buf, size_t* len) {
	size_t msglen;

	do_recv(buf, 2);
	msglen = ((unsigned)buf[0] << 8) | (unsigned)buf[1];

	if(msglen > 1024) {
		fprintf(stdout, "Message too long (%d)!\n", (int)msglen);
		exit(2);
	}

	do_recv(buf, msglen);
	*len = msglen;
}

bool check_msg(const unsigned char* key, unsigned char* msg, size_t* msglen) {
	HMAC_CTX hmac;
	size_t in_length = *msglen;
	unsigned char digest[32];

	HMAC_CTX_init(&hmac);
	HMAC_Init(&hmac, key, 32, EVP_sha256());
	HMAC_Update(&hmac, msg, in_length-32);
	HMAC_Final(&hmac, digest, 0);
	HMAC_CTX_cleanup(&hmac);

	*msglen = in_length - 32;
	return memcmp(digest, &msg[in_length - 32], 32) == 0;
}

static void decrypt_msg(cipher_state *cipher, unsigned char *msg, size_t msglen) {
	AES_ctr128_encrypt(msg, msg, msglen, &cipher->key, cipher->ctr, cipher->ecount_buf, &cipher->num);
}

void doPutFlag(const std::string& ip, const std::string& flag) {
	unsigned char buffer[51];
	unsigned char key[32];
	unsigned char id[8];
	cipher_state state;

	buffer[0] = 'N'; buffer[1] = 'E'; buffer[2] = 'W'; 
	RAND_bytes(&buffer[3], 48);
	memset(&state, 0, sizeof(state));
	memcpy(&key[0], &buffer[3], 32);
	memcpy(state.ctr, &buffer[35], 16);

	AES_set_encrypt_key(key, 256, &state.key);
	
	do_send(buffer, 51);
	do_recv(id, 8);

	unsigned char msgbuf[2048];
	msgbuf[0] = 'V';
	memcpy(msgbuf+1, id, 8);
	msgbuf[9] = 0;
	msgbuf[10] = 16;
	memcpy(msgbuf+11, state.ctr, 16);
	expectedMessages.push_back(buf(msgbuf, msgbuf+27));

	size_t msglen;
	std::string msg = "plant evidence"+flag;

	encrypt_msg(msgbuf+9, &state, key, (const unsigned char*)msg.c_str(), msg.length(), &msglen);
	do_send(msgbuf+9, msglen);

	msgbuf[0] = 'I';
	expectedMessages.push_back(buf(msgbuf, msgbuf+(msglen+9)));

	do_recv_msg(msgbuf+11, &msglen);
	msgbuf[0] = 'O';
	memcpy(msgbuf+1, id, 8);
	msgbuf[9] = (unsigned char)(msglen>>8);
	msgbuf[10] = (unsigned char)(msglen);
	expectedMessages.push_back(buf(msgbuf, msgbuf+11+msglen));

	if(!check_msg(key, msgbuf+11, &msglen)) {
		fprintf(stdout, "Invalid message signature!\n");
		exit(2);
	}

	decrypt_msg(&state, msgbuf+11, msglen);

	if(flag.length() != msglen || memcmp(msgbuf+11, flag.c_str(), msglen)) {
		fprintf(stdout, "Evidence was not echo'ed correctly!\n");
		exit(2);
	}
	
	void* dummy;
	setMainDone();
	pthread_join(loggerThread, &dummy);

	IdAndKey data;
	memcpy(data.key, key, 32);
	memcpy(&data.id, id, 8);
	setIDAndKey((ip+flag).c_str(), data);
}

void doGetFlag(const std::string& ip, const std::string& flag) {
	unsigned char buffer[27];
	
	buffer[0] = 'O'; buffer[1] = 'L'; buffer[2] = 'D';
	IdAndKey k = getIDAndKey((ip+flag).c_str());
	memcpy(&buffer[3], &k.id, 8);
	RAND_bytes(&buffer[11], 16);

	do_send(buffer, 27);

	std::string msg = "extract evidence";
	unsigned char msgbuf[2048];
	size_t msglen;

	cipher_state state;
	memset(&state, 0, sizeof(state));
	memcpy(state.ctr, &buffer[11], 16);
	AES_set_encrypt_key(k.key, 256, &state.key);
	encrypt_msg(msgbuf+9, &state, k.key, (const unsigned char*)msg.c_str(), msg.length(), &msglen);
	msgbuf[0] = 'I';
	memcpy(msgbuf+1, &k.id, 8);
	do_send(msgbuf+9, msglen);
	expectedMessages.push_back(buf(msgbuf, msgbuf+9+msglen));

	do_recv_msg(msgbuf+11, &msglen);
	msgbuf[0] = 'O';
	msgbuf[9] = (unsigned char)(msglen>>8);
	msgbuf[10] = (unsigned char)(msglen);
	expectedMessages.push_back(buf(msgbuf, msgbuf+11+msglen));

	if(!check_msg(k.key, msgbuf+11, &msglen)) {
		fprintf(stdout, "Invalid message signature!\n");
		exit(2);
	}

	decrypt_msg(&state, msgbuf+11, msglen);

	if(flag.length() != msglen || memcmp(msgbuf+11, flag.c_str(), msglen)) {
		fprintf(stdout, "Invalid flag returned!\n");
		exit(2);
	}
}

void putFlag(const std::string& ip, const std::string& flag) {
	if(flag.length() > 512) {
		fprintf(stderr, "Flag too long!\n");
		exit(3);
	}

	setupConnection(ip);
	startLoggerThread();
	doPutFlag(ip, flag);
}

void getFlag(const std::string& ip, const std::string& flag) {
	if(flag.length() > 512) {
		fprintf(stderr, "Flag too long!\n");
		exit(3);
	}

	void* dummy;
	setupConnection(ip);
	startLoggerThread();
	doGetFlag(ip, flag);
	setMainDone();
	pthread_join(loggerThread, &dummy);
}

void* global_timeout_func(void* arg) {
	int r;
	struct timespec val, rem;

	val.tv_sec = GLOBAL_TIMEOUT / 1000;
	val.tv_nsec = (GLOBAL_TIMEOUT % 1000) * 1000;

	while((r = nanosleep(&val, &rem)) < -1 && errno == EINTR) { errno = 0; val = rem; }

	if(r < 0) {
		perror("nanosleep");
		exit(3);
	}

	fprintf(stdout, "Global timeout (35 s) occurred!\n");
	exit(2);
}

int main(int argc, const char* argv[]) {
	pthread_t global_timeout;

	if(argc != 4) {
		fprintf(stderr, "Missing/too many command line arguments!\n");
		return 3;
	}

	SSL_load_error_strings();
	SSL_library_init();
	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();
	signal(SIGUSR1, SIG_IGN);

	if(pthread_create(&global_timeout, 0, global_timeout_func, 0) < 0) {
		perror("pthread_create");
		exit(3);
	}

	pthread_detach(global_timeout);

	if(std::string(argv[1]) == "put") {
		putFlag(argv[2], argv[3]);
	} else if(std::string(argv[1]) == "get") {
		getFlag(argv[2], argv[3]);
	} else {
		fprintf(stderr, "Unknown command '%s'!\n", argv[1]);
		return 3;
	}

	return 0;
}
