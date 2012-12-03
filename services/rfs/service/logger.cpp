#include "logger.h"
#include <algorithm>
#include <cstdio>
#include <algorithm>
#include <stdexcept>
#include <exception>
#include <vector>
#include "net.h"
#include "globals.h"

using namespace std;

void* Logger::acc_entry_point(void* logger) {
	((Logger*)logger)->accept_main();
	return 0;
}

void* Logger::log_entry_point(void* logger) {
	((Logger*)logger)->log_main();
	return 0;
}

Logger::Logger() :
	acceptor_thread(), logger_thread(), log_cond(), acc_mut(), log_mut(), stop(0), messages(), clients(), listener(-1)
{
	pthread_cond_init(&log_cond, 0);
	pthread_mutex_init(&acc_mut, 0);
	pthread_mutex_init(&log_mut, 0);

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0) {
		perror("socket");
		throw runtime_error("Could not create listener socket for logging!");
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(LOG_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(listener, (const sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(listener);
		throw runtime_error("Could not bind listener socket for logging!");
	}

	int arg = 1;
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(int)) != 0) {
		perror("Warning - setsockopt failed");
	}

	if(listen(listener, SOMAXCONN) < 0) {
		perror("listen");
		close(listener);
		throw runtime_error("Could not listen on logger socket!");
	}

	if(pthread_create(&acceptor_thread, 0, Logger::acc_entry_point, this) != 0) {
		perror("pthread_create");
		close(listener);
		throw runtime_error("Could not create acceptor thread!");
	}

	if(pthread_create(&logger_thread, 0, Logger::log_entry_point, this) != 0) {
		perror("pthread_create");
		close(listener);
		throw runtime_error("Could not create logger thread!");
	}
}

Logger::~Logger() {
	void* dummy;

	pthread_mutex_lock(&acc_mut);
	pthread_mutex_lock(&log_mut);
	stop = 1;

	pthread_cond_signal(&log_cond);
	pthread_mutex_unlock(&log_mut);
	pthread_mutex_unlock(&acc_mut);

	pthread_join(acceptor_thread, &dummy);
	pthread_join(logger_thread, &dummy);

	close(listener);
}

void Logger::logMessage(char inout, client_id_t id, const unsigned char* msg, size_t length) {
	ustring message(11 + length, 0);

	message[0]  = (unsigned char)inout;
	message[1]  = (unsigned char)(id >> 56);
	message[2]  = (unsigned char)(id >> 48);
	message[3]  = (unsigned char)(id >> 40);
	message[4]  = (unsigned char)(id >> 32);
	message[5]  = (unsigned char)(id >> 24);
	message[6]  = (unsigned char)(id >> 16);
	message[7]  = (unsigned char)(id >>  8);
	message[8]  = (unsigned char)(id >>  0);
	message[9]  = (unsigned char)(length >> 8);
	message[10] = (unsigned char)(length >> 0);
	copy(msg, msg+length, message.begin()+11);

	pthread_mutex_lock(&log_mut);
	messages.push(message);
	pthread_cond_signal(&log_cond);
	pthread_mutex_unlock(&log_mut);
}

void Logger::accept_main() {
	fd_set rfds;

	while(!stop) {
		struct timeval tv;
		int s;

		tv.tv_sec = 0;
		tv.tv_usec = 250000;

		FD_ZERO(&rfds);
		FD_SET(listener, &rfds);
		s = select(listener+1, &rfds, 0, 0, &tv);

		if(s > 0) {
			int newcl = accept(listener, 0, 0);
			
			if(newcl < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
				perror("accept");

				if(errno == ECONNABORTED || errno == EINTR || errno == EMFILE || errno == ENFILE || errno == ENOBUFS || errno == ENOMEM || errno == ETIMEDOUT) {
					continue;
				} else {
					throw runtime_error("Accepting connections failed for logger socket!");
				}
			}

			set_nonblocking(newcl);

			pthread_mutex_lock(&acc_mut); 
			clients.insert(newcl);
			pthread_mutex_unlock(&acc_mut);
		} else if(s < 0 && errno != EINTR) {
			perror("select");
			throw runtime_error("Could not select");
		}
	}
}

void Logger::log_main() {
	pthread_mutex_lock(&log_mut);

	while(!stop) {
		pthread_cond_wait(&log_cond, &log_mut);

		if(!stop) {
			while(!messages.empty()) {
				// fetch next message from queue
				ustring msg; 
				msg.swap(messages.front());
				messages.pop();
				const size_t msglen = msg.length();
				pthread_mutex_unlock(&log_mut);
			
				// fetch list of clients
				pthread_mutex_lock(&acc_mut);
				vector<int> cls(clients.begin(), clients.end());
				pthread_mutex_unlock(&acc_mut);

				// vector containing the amount of data sent to each client
				vector<size_t> sent(cls.size(), 0);

				// vector of broken clients
				vector<int> broken;

				// as long as there are some clients that we're not done with
				while((size_t)count(sent.begin(), sent.end(), msglen) < cls.size()) {
					// wait for write-readiness
					struct timeval tv;
					tv.tv_sec =  LOG_SEND_TIMEOUT / 1000000;
					tv.tv_usec = LOG_SEND_TIMEOUT % 1000000;
					fd_set wfds;
					FD_ZERO(&wfds);
					int m = -1;
					for(size_t i = 0; i < cls.size(); i++) {
						if(sent[i] < msglen) {
							FD_SET(cls[i], &wfds);
							if(cls[i] > m) m = cls[i];
						}
					}

					// wait for progress
					int r = select(m+1, 0, &wfds, 0, &tv);
					if(r < 0) {
						if(errno != EBADF) {
							// unless this is just a broken FD,
							// log an error; otherwise just check
							// all FDs
							perror("log_main: select");
						}
					} else if(r == 0) {
						// the leftover FDs had a timeout; shut them down
						for(size_t i = 0; i < cls.size(); i++) {
							if(sent[i] < msglen) {
								broken.push_back(i);
							}
						}

						break;
					}

					// try sending data to them
					for(size_t i = 0; i < cls.size(); i++) {
						ssize_t s;

						while(sent[i] < msglen) {
							s = send(cls[i], &msg[sent[i]], msglen - sent[i], MSG_NOSIGNAL);

							if(s <= 0) {
								if(errno == EAGAIN || errno == EWOULDBLOCK) {
									break;
								} else {
									sent[i] = msglen;
									broken.push_back(cls[i]);
								}
							} else {
								sent[i] += s;
							}
						}
					}
				}

				// remove broken clients
				pthread_mutex_lock(&acc_mut);
				for(size_t i = 0; i < broken.size(); i++) {
					close(broken[i]);
					clients.erase(broken[i]);
				}
				pthread_mutex_unlock(&acc_mut);

				pthread_mutex_lock(&log_mut);
			}
		}
	}

	pthread_mutex_unlock(&log_mut);
}

Logger Logger::logger;
