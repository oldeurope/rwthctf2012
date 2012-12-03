#ifndef __STATE_H_INCLUDED__
#define __STATE_H_INCLUDED__

#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <pthread.h>

typedef uint64_t client_id_t;

class State {
	private:
		std::string evidence;
		unsigned char key[256 / 8];
		client_id_t id;
		struct timeval last_use;
		pthread_mutex_t mut;

	public:
		State();
		State(client_id_t id, const unsigned char* key);
		~State();

		bool getEvidence(std::string &output);
		bool setEvidence(const std::string& input);

		void lock();
		bool trylock();
		void unlock();

		void setReused();
		const struct timeval &getLastUse() const;

		client_id_t getID() const;
		const unsigned char* getKey() const;
};

#endif
