#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include "rpc.h"
#include "store.h"


rpcresult_t *simple_result_ptr;

#define MAX_FETCH_KEYSIZE 128

rpcresult_t *kvs_put_handler(rpcrequest_t *req) {
	kvs_put_req_t *putreq = (kvs_put_req_t*)req;
	simple_result_ptr->resultCode = store_put(putreq->kvstore, putreq->key, putreq->value);
	simple_result_ptr->request = req;
	return simple_result_ptr;
}

rpcresult_t *kvs_get_handler(rpcrequest_t *req) {
	kvs_get_result_t* res = (kvs_get_result_t*) malloc(sizeof(kvs_get_result_t));
	res->res.request = req;
	res->value = malloc(MAX_FETCH_KEYSIZE);
	res->res.resultCode = store_get(((kvs_get_req_t*)req)->kvstore, ((kvs_get_req_t*)req)->key, res->value, MAX_FETCH_KEYSIZE);
	return (rpcresult_t*)res;
}

rpcresult_t *kvs_list_handler(rpcrequest_t *req) {
	kvs_list_result_t* res = (kvs_list_result_t*) malloc(sizeof(kvs_list_result_t));
	res->res.request = req;
	res->keys = store_list(((kvs_list_req_t*)req)->kvstore);
	res->res.resultCode = 0;
	return (rpcresult_t*)res;
}

rpcresult_t *close_socket(rpcrequest_t *req) {
	close_sock_call_t *closeReq = (close_sock_call_t*)req;
	int res = close(closeReq->sockid);
	simple_result_ptr->request = req;
	simple_result_ptr->resultCode = res;
	return simple_result_ptr;
}

const rpchandler_t rpc_handlers[] = {
	&kvs_put_handler,
	&kvs_get_handler,
	&kvs_list_handler,
	&close_socket
};

rpcresult_t *rpccall(int rpcsock, rpcrequest_t* req) {
	int res;
	res = write(rpcsock,&req,sizeof(req));
	if (res != sizeof(req)) {
		return NULL;
	}
	rpcrequest_t *result;
	res = read(rpcsock, &result, sizeof(result));
	if (res != sizeof(req)) {
		return NULL;
	}
	return (rpcresult_t*)result;
}



void *rpcserver(void *data) {
	int sockid = (int)(long int)data;
	rpcresult_t simple_result;
	simple_result_ptr = &simple_result;
	rpcresult_t invalid_call = {
		.request = NULL,
		.resultCode = (int)0xDEADC0DELL
	};

	store_init();

	while (1) {
		rpcrequest_t *req;
		rpcresult_t *res;
		int rc;
		rc = read(sockid,&req,sizeof(req));
		if (rc <= 0)
			break; // broken pipe
		if (rc != sizeof(req)) {
			continue; // ignore this request
		}
		if (req->callid < 0 || req->callid >= sizeof(rpc_handlers)) {
			res = &invalid_call;
			invalid_call.request = req;
		}
		else if (req->callid == 0) {
			// terminate rpc server
			close(sockid);
			break;
		}
		else {
			res = rpc_handlers[req->callid-1](req);
		}
		if (write(sockid,&res,sizeof(res)) != sizeof(res))
		{
			printf("\x1B[37;1m[\x1B[31;1mError\x1B[37;1m]\x1B[0m Write failed.\n");
			break;
		}
	}

	printf("\x1B[37;1m[\x1B[34mInfo\x1B[37;1m]\x1B[0m RPC server stopped\n");
//	pthread_exit(NULL);
	return NULL;
}


int init_rpc() {
	int sockets[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
		printf("\x1B[37;1m[\x1B[31;1mError\x1B[37;1m]\x1B[0m Can't create RPC socketpair.\n");
		exit(-1);
	}
	pthread_t serverThread;
	if (pthread_create(&serverThread, NULL, &rpcserver, (void*)(long int)sockets[1])) {
		printf("\x1B[37;1m[\x1B[31;1mError\x1B[37;1m]\x1B[0m Can't create RPC server thread.\n");
		exit(-1);
	}
	pthread_detach(serverThread);

	// lock down this thread
	if (prctl(PR_SET_SECCOMP,1, 0, 0, 0) != 0) {
		printf("\x1B[37;1m[\x1B[31;1mError\x1B[37;1m]\x1B[0m Can't enter sandbox mode.\n");
	}
	else {
		printf("\x1B[37;1m[\x1B[34mInfo\x1B[37;1m]\x1B[0m Entering sandbox mode.\n");
	}
	return sockets[0];
}
