#ifndef __LOGGER_H_INCLUDED__
#define __LOGGER_H_INCLUDED__

#include <set>
#include <queue>
#include <string>
#include <set>
#include <pthread.h>
#include "state.h"

/**
 * Implements the logger threads that
 * the logging interface of this RFS
 * will connect to.
 * It sends every valid message sent/recvd on the
 * main channel to all connected clients without
 * decrypting it.
 * This does not apply to key exchange messages.
 */
class Logger {
	private:
		typedef std::basic_string<unsigned char> ustring;

		pthread_t acceptor_thread, logger_thread;
		pthread_cond_t log_cond;
		pthread_mutex_t acc_mut, log_mut;
		volatile long stop;
		std::queue<ustring> messages;
		std::set<int> clients;
		int listener;

		static void* acc_entry_point(void* logger);
		static void* log_entry_point(void* logger);

		void accept_main();
		void log_main();

		Logger();

	public:
		static Logger logger;

		~Logger();

		void logMessage(char inout, client_id_t id, const unsigned char* msg, size_t length);
};

#endif
