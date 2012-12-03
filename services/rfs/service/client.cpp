#include "client.h"
#include "net.h"
#include "globals.h"
#include "logger.h"
#include <stdint.h>
#include <cstring>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

void* Client::client_entry_point(void* client) {
	((Client*)client)->client_main();
	return 0;
}

Client::Client(IDBase& base, int _sock) : ids(base), sock(_sock) {
	pthread_t dummy;

	if(pthread_create(&dummy, 0, client_entry_point, this) != 0) {
		close(_sock);
		sock = -1;
		throw errno;
	}

	pthread_detach(dummy);
}

Client::~Client() {
	if(sock != -1) {
		close(sock);
	}
}

typedef struct aes_ctr_state {
	AES_KEY key;
	uint8_t ctr[16], ecount_buf[16];
	unsigned num;
} cipher_state;

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

void Client::session(client_id_t id) {
	// receive the IV
	cipher_state cipher;
	unsigned char ivbuf[16];
	unsigned char mbuf[MAX_LENGTH];
	size_t msglen;

	do_recv(sock, ivbuf, 16);

	State* state = ids.lock(id);
	if(state == 0) {
		close(sock);
		throw ENOENT;
	}

	Logger::logger.logMessage('V', id, ivbuf, 16);

	memset(&cipher, 0, sizeof(cipher));

	AES_set_encrypt_key(state->getKey(), 256, &cipher.key);
	memcpy(cipher.ctr, ivbuf, 16);

	state->unlock();
	
	while(true) {
		do_recv_msg(sock, mbuf, &msglen);

		State* state = ids.lock(id);
		if(state == 0) {
			close(sock);
			throw ENOENT;
		}

		if(!check_msg(state->getKey(), mbuf, &msglen)) {
			// invalid message. send a nice fuck-you message and disconnect.
			// do not log this message
			
			encrypt_msg(mbuf, &cipher, state->getKey(), FUCK_YOU_MESSAGE, FUCK_YOU_LENGTH, &msglen);
			state->unlock();

			do_send(sock, mbuf, msglen);

			close(sock);
			throw EINVAL;
		}

		// log this message
		Logger::logger.logMessage('I', id, mbuf, msglen+32);

		// decrypt it
		decrypt_msg(&cipher, mbuf, msglen);

		// handle it
		if(strstr((const char*)mbuf, (const char*)PLANT_EVIDENCE_MESSAGE) == (const char*)mbuf) {
			std::string evidence((const char*)&mbuf[PLANT_EVIDENCE_LENGTH], msglen - PLANT_EVIDENCE_LENGTH);

			state->setEvidence(evidence);
			encrypt_msg(mbuf, &cipher, state->getKey(), (const unsigned char*)evidence.c_str(), evidence.length(), &msglen);
			state->unlock();

			do_send(sock, mbuf, msglen);
			Logger::logger.logMessage('O', id, mbuf+2, msglen-2);
			continue;
		} else if(strstr((const char*)mbuf, (const char*)EXTRACT_EVIDENCE_MESSAGE) == (const char*)mbuf) {
			std::string evi;
			
			if(!state->getEvidence(evi)) {
				state->unlock();
				close(sock);
				throw EINVAL;
			}

			encrypt_msg(mbuf, &cipher, state->getKey(), (const unsigned char*)evi.c_str(), evi.length(), &msglen);
			state->setReused();
			state->unlock();

			do_send(sock, mbuf, msglen);
			Logger::logger.logMessage('O', id, mbuf+2, msglen-2);
			continue;
		}

		state->setReused();
		state->unlock();
	}
}

void Client::client_main() {
	unsigned char tbuf[3];
	unsigned char ibuf[8];
	unsigned char kbuf[32];
	client_id_t id;
	bool old;

	try {
		// retrieve the type of this session
		do_recv(sock, tbuf, 3);

		if(memcmp(tbuf, "OLD", 3) == 0) {
			old = true;
		} else if(memcmp(tbuf, "NEW", 3) == 0) {
			old = false;
		} else {
			close(sock);
			throw 0;
		}

		State *state;
		
		if(old) {
			// receive the ID
			do_recv(sock, ibuf, 8);
			id = (((client_id_t)ibuf[0]) << 56) + 
			     (((client_id_t)ibuf[1]) << 48) +
			     (((client_id_t)ibuf[2]) << 40) +
			     (((client_id_t)ibuf[3]) << 32) +
			     (((client_id_t)ibuf[4]) << 24) +
			     (((client_id_t)ibuf[5]) << 16) +
			     (((client_id_t)ibuf[6]) <<  8) +
			     (((client_id_t)ibuf[7]));
		} else {
			// receive key
			do_recv(sock, kbuf, 32);

			State &newState = ids.addNew(kbuf);
			id = newState.getID();
			newState.unlock();

			ibuf[0] = (unsigned char)((id >> 56) & 0xff);
			ibuf[1] = (unsigned char)((id >> 48) & 0xff);
			ibuf[2] = (unsigned char)((id >> 40) & 0xff);
			ibuf[3] = (unsigned char)((id >> 32) & 0xff);
			ibuf[4] = (unsigned char)((id >> 24) & 0xff);
			ibuf[5] = (unsigned char)((id >> 16) & 0xff);
			ibuf[6] = (unsigned char)((id >>  8) & 0xff);
			ibuf[7] = (unsigned char)((id >>  0) & 0xff);

			// send id
			do_send(sock, ibuf, 8);
		}

		// is this ID available?
		state = ids.lock(id);
			
		// this is not a valid id!
		if(state == 0) {
			close(sock);
			throw 0;
		}

		state->setReused();
		state->unlock();
		session(id);
	} catch(...) {
		sock = -1;
	}

	delete this;
}
