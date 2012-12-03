#ifndef __CLIENT_H_INCLUDED__
#define __CLIENT_H_INCLUDED__
#include "id-base.h"

class Client {
	private:
		IDBase &ids;
		int sock;

		static void* client_entry_point(void* client);
		void client_main();
		void session(client_id_t id);

	public:
		Client(IDBase &base, int sock);
		~Client();
};

#endif
