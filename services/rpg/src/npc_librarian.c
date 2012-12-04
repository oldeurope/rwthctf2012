#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "npc_librarian.h"
#include "game_engine.h"
#include "libmudcom/map.h"
#include "rpc.h"

const char librarian_greeting[] = "Welcome to our beautiful library, the heaven of books. I can serve you with the latest and greatest publications. You can also give me your publication to deposit it here.";
const char librarian_question[] = "How can I serve you?";
const char librarian_fetch_name[] = "Please give me the name of the publication:";
const char librarian_deposit_key[] = "What is the name of your publication?";
const char librarian_deposit_value[] = "What is the content of your publication?";
const char librarian_nothing[] = "Have a nice day!";
const char librarian_deposit_happy[] = "It feels so good to have another great publication here. It *is* great, right?";
const char librarian_deposit_sad[] = "Sorry, but our library is currently completely full.";
const char librarian_fetch_sad[] = "Sorry, I was not able to find this publication.";


char librarian_key_buf[128];

void librarian_fetch(int fd, gamestate_t *state, void *data, const char *input) {
	kvs_get_req_t req;
	req.req.callid = RPC_CALL_KVS_GET;
	req.kvstore = "librarian";
	req.key = input;
	kvs_get_result_t *res = (kvs_get_result_t*)rpccall(state->rpcfd, (rpcrequest_t*)&req);
	if (res->res.resultCode) {
		char buf[128];
		snprintf(buf, sizeof(buf), "Here is your publication:\n%s", res->value);
		show_multiline_text(fd, buf, librarian.npcdesc);
	}
	else {
		show_multiline_text(fd, librarian_fetch_sad, librarian.npcdesc);
	}
	free(res->value);
	free(res);
}

void librarian_deposit(int fd, gamestate_t *state, void *data, const char *input) {
	if (librarian_key_buf[0] == 0)
	{
		strncpy(librarian_key_buf, input, sizeof(librarian_key_buf));
		start_textinput(fd,state,librarian_deposit_value,librarian.npcdesc,&librarian_deposit,(void*)state);
	}
	else
	{
		kvs_put_req_t req;
		req.req.callid = RPC_CALL_KVS_PUT;
		req.kvstore = "librarian";
		req.key = librarian_key_buf;
		req.value = input;
		rpcresult_t *res = rpccall(state->rpcfd, (rpcrequest_t*)&req);
		if (res->resultCode) {
			show_multiline_text(fd, librarian_deposit_happy, librarian.npcdesc);
		}
		else {
			show_multiline_text(fd, librarian_deposit_sad, librarian.npcdesc);
		}
		librarian_key_buf[0] = 0;
	}
}

void librarian_reply(int fd, gamestate_t *state, void *data, int answer) {
	if (answer == 1)
	{
		start_textinput(fd,state,librarian_fetch_name,librarian.npcdesc,&librarian_fetch,NULL);
	}
	else if (answer == 0)
	{
		librarian_key_buf[0] = 0;
		start_textinput(fd,state,librarian_deposit_key,librarian.npcdesc,&librarian_deposit,NULL);
	}
	else show_multiline_text(fd, librarian_nothing, librarian.npcdesc);
}

const mcdialog_t librarian_dialog = {
	.text = {
		"I wish to deposit a publication.",
		"I would like to fetch a publication.",
		"Hm. Maybe I'll come back later."
	},
	.callback = &librarian_reply
};


void handle_librarian(int fd, gamestate_t *state) {
	show_multiline_text(fd, librarian_greeting, librarian.npcdesc);
	start_mcinput(fd,state,&librarian_dialog,librarian_question,librarian.npcdesc,NULL);
}


npcstruct_t librarian = {
	.label = 'L',
	.npcdesc = "Meticulous Librarian",
	.npcfunc = &handle_librarian,
	.next = 0
};
