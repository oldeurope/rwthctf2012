#include "state.h"
#include <cstring>

State::State() {}

State::State(client_id_t _id, const unsigned char* _key) : evidence(), key(), id(_id), last_use(), mut() {
	pthread_mutex_init(&mut, 0);
	memcpy(key, _key, 256 / 8);
	gettimeofday(&last_use, 0);
}

State::~State() {
	pthread_mutex_destroy(&mut);
}

void State::lock() {
	pthread_mutex_lock(&mut);
}

bool State::trylock() {
	return pthread_mutex_trylock(&mut) == 0;
}

void State::unlock() {
	pthread_mutex_unlock(&mut);
}

bool State::getEvidence(std::string &output) {
	if(!evidence.empty()) {
		setReused();
		output.assign(evidence);
		return true;
	}

	return false;
}

bool State::setEvidence(const std::string& input) {
	if(evidence.empty()) {
		setReused();
		evidence.assign(input);
		return true;
	}

	return false;
}

void State::setReused() {
	gettimeofday(&last_use, 0);
}

const struct timeval &State::getLastUse() const {
	return last_use;
}

client_id_t State::getID() const {
	return id;
}

const unsigned char *State::getKey() const {
	return key;
}
