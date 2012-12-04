#ifndef RPC_H
#define RPC_H

#include <sys/socket.h>
#include <unistd.h>

struct rpcrequest {
	int callid;
};

typedef struct rpcrequest rpcrequest_t;

struct kvs_put_req {
	rpcrequest_t req;
	const char* kvstore;
	const char* key;
	const char* value;
};
typedef struct kvs_put_req kvs_put_req_t;

struct kvs_get_req {
	rpcrequest_t req;
	const char* kvstore;
	const char* key;
};
typedef struct kvs_get_req kvs_get_req_t;

struct kvs_list_req {
	rpcrequest_t req;
	const char* kvstore;
};
typedef struct kvs_list_req kvs_list_req_t;


struct rpcresult {
	rpcrequest_t *request;
	int resultCode;
};
typedef struct rpcresult rpcresult_t;

struct kvs_get_result {
	rpcresult_t res;
	char* value;
};
typedef struct kvs_get_result kvs_get_result_t;

struct kvs_list_result {
	rpcresult_t res;
	char *keys;
	const int len;
};
typedef struct kvs_list_result kvs_list_result_t;


struct close_sock_call {
	rpcrequest_t req;
	int sockid;
};
typedef struct close_sock_call close_sock_call_t;

typedef rpcresult_t* (*rpchandler_t)(rpcrequest_t *);

rpcresult_t *rpccall(int rpcsock, rpcrequest_t* req);
int init_rpc();

#define RPC_CALL_END_SERVICE 0
#define RPC_CALL_KVS_PUT 1
#define RPC_CALL_KVS_GET 2
#define RPC_CALL_KVS_LIST 3
#define RPC_CALL_CLOSE_SOCK 4


#endif
