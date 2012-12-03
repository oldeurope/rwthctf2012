#ifndef __NET_H_INCLUDED__
#define __NET_H_INCLUDED__

#define MAX_LENGTH 1024

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <exception>

static inline size_t do_send(int sock, const void* buf, size_t length) {
	ssize_t s;
	size_t sent = 0;

	while(sent < length) {
		s = send(sock, &((const unsigned char*)buf)[sent], length - sent, MSG_NOSIGNAL);

		if(s <= 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				return sent;
			} else {
				close(sock);
				throw errno;
			}
		}

		sent += s;
	}

	return sent;
}

static inline size_t do_recv(int sock, void* buf, size_t length) {
	ssize_t r;
	size_t recvd = 0;

	while(recvd < length) {
		r = recv(sock, &((unsigned char*)buf)[recvd], length - recvd, 0);

		// End of stream
		if(r == 0) {
			close(sock);
			errno = 0;
			throw 0;
		} else if(r < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				return recvd;
			}

			close(sock);
			throw errno;
		}

		recvd += r;
	}

	return recvd;
}

static inline void do_recv_msg(int sock, void* buf, size_t *length) {
	unsigned char sbuf[2];
	size_t l;

	do_recv(sock, sbuf, 2);
	l = (((unsigned)sbuf[0]) << 8) | ((unsigned)sbuf[1]);
	if(l > MAX_LENGTH || l < 32) {
		close(sock);
		throw EINVAL;
	}
	
	do_recv(sock, buf, l);
	*length = l;
}

static inline void set_nonblocking(int sock) {
	int flags = fcntl(sock, F_GETFL);

	if(flags == -1) {
		perror("fcntl");
		throw std::runtime_error("Could not fetch file descriptor flags!");
	}

	flags |= O_NONBLOCK;

	if(fcntl(sock, F_SETFL, flags) == -1) {
		perror("fcntl");
		throw std::runtime_error("Could not set non-blocking mode!");
	}
}

#endif
