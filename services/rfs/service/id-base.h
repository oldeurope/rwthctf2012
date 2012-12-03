#ifndef __ID_BASE_H_INCLUDED__
#define __ID_BASE_H_INCLUDED__

#include <map>
#include <pthread.h>
#include <stdint.h>

#include "state.h"

/**
 * How many seconds to keep unused IDs in the database.
 */
static const uint32_t GC_ID_TIMEOUT = 360;

/**
 * Controls the time (in seconds) between two GC runs.
 */
static const uint32_t GC_INTERVAL = 15;

/**
 * Maps IDs as sent by remote hosts
 * to encryption keys and evidence
 * collected on their behalf.
 */
class IDBase {
	private:
		pthread_t gc;
		pthread_cond_t gc_cond;
		volatile long gc_stop;

		pthread_mutex_t mut;
		std::map<client_id_t, State> tbl;

		void gc_main();
		static void* gc_entry_point(void* idBase);

	public:
		IDBase();
		~IDBase();

		/**
		 * Atomically creates a new entry in this map.
		 * Takes a 256 bit AES key.
		 * Returns a reference to a locked State.
		 */
		State &addNew(const unsigned char* key);

		/**
		 * Fetches and locks a specific ID.
		 * Returns NULL on failure.
		 */
		State *lock(client_id_t id);
};

#endif
