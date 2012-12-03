#include "id-base.h"
#include <time.h>
#include <cstdio>
#include <stdexcept>
#include <exception>
#include <openssl/rand.h>

void* IDBase::gc_entry_point(void* idBase) {
	((IDBase*)idBase)->gc_main();
	return 0;
}

void IDBase::gc_main() {
	struct timespec spec;

	pthread_mutex_lock(&mut);
	
	do {
		clock_gettime(CLOCK_REALTIME, &spec);
		spec.tv_sec += GC_INTERVAL;
		pthread_cond_timedwait(&gc_cond, &mut, &spec);

		if(!gc_stop) {
			struct timeval cur;
			gettimeofday(&cur, 0);

			// perform garbage collection
			for(std::map<client_id_t, State>::iterator i = tbl.begin(); i != tbl.end();) {
				if(i->second.trylock()) {
					if(cur.tv_sec - i->second.getLastUse().tv_sec >= GC_ID_TIMEOUT) {
						i->second.unlock();
						tbl.erase(i++);
					} else {
						i->second.unlock();
						++i;
					}
				}
			}
		}
	} while(!gc_stop);
	
	pthread_mutex_unlock(&mut);
}

IDBase::IDBase() :
	gc(), gc_cond(), gc_stop(0), mut(), tbl()
{
	pthread_mutex_init(&mut, 0);
	pthread_cond_init(&gc_cond, 0);
	
	if(pthread_create(&gc, 0, IDBase::gc_entry_point, this) != 0) {
		perror("pthread_create");
		throw std::runtime_error("Failed to create GC thread!");
	}
}

IDBase::~IDBase() {
	void* dummy;

	pthread_mutex_lock(&mut);
	gc_stop = 1;
	pthread_cond_signal(&gc_cond);
	pthread_mutex_unlock(&mut);
	pthread_join(gc, &dummy);
	pthread_mutex_destroy(&mut);
	pthread_cond_destroy(&gc_cond);
}

State &IDBase::addNew(const unsigned char* key) {
	State *result;
	client_id_t id;

	pthread_mutex_lock(&mut);
	
	while(true) {
		RAND_bytes((unsigned char*)&id, sizeof(id));
		if(!tbl.count(id)) {
			result = &(tbl[id] = State(id, key));
			result->lock();
			pthread_mutex_unlock(&mut);
			return *result;
		}
	}
}

State *IDBase::lock(client_id_t id) {
	State* result = 0;
	std::map<client_id_t, State>::iterator i;

	pthread_mutex_lock(&mut);

	if((i = tbl.find(id)) != tbl.end()) {
		result = &i->second;
		result->lock();
	}

	pthread_mutex_unlock(&mut);

	return result;
}
