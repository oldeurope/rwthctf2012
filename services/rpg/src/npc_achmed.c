#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "npc_achmed.h"
#include "game_engine.h"
#include "libmudcom/map.h"
#include "rpc.h"

const char achmed_greeting[] = "Hello my friend, I'm Honest Achmed and I sell all kinds of certificates to anyone willing to pay me 15 bucks.";
const char achmed_question[] = "So tell me, are you interested? You'll get the first one for only 10 gold! (y/n)";
const char achmed_no[] = "Alright, see you later.";
const char achmed_honesty[] = "\"Honest Achmed\", that's what they call me...";
const char achmed_nogold[] = "I only offer loans for my cousins. Well, except my cousin Mustafa...";
const char achmed_enter_key[] = "Ok. Which name should your secret have?";
const char achmed_enter_value[] = "... and what's your secret?";
const char achmed_happy[] = "Fine. Your secret will not be revealed";
const char achmed_sad[] = "Sorry - my box of secrets is full";

char achmed_key_buf[128];
char achmed_value_buf[128];


void achmed_value_callback(int fd, gamestate_t *state, void *data, const char *input) {
//	printf("value callback called with %s\n",input);
	kvs_put_req_t req;
	req.req.callid = RPC_CALL_KVS_PUT;
	req.kvstore = "achmed";
	req.key = achmed_key_buf;
	req.value = input;
	rpcresult_t *res = rpccall(state->rpcfd, (rpcrequest_t*)&req);
	if (res->resultCode) {
		show_multiline_text(fd, achmed_happy, achmed.npcdesc);
	}
	else {
		show_multiline_text(fd, achmed_sad, achmed.npcdesc);
	}
}

void achmed_key_callback(int fd, gamestate_t *state, void *data, const char *input) {
//	printf("key callback called with %s\n", input);
	strncpy(achmed_key_buf,input,sizeof(achmed_key_buf));
	start_textinput(fd,state,achmed_enter_value,achmed.npcdesc, &achmed_value_callback, NULL);
//	printf("setting up next callback complete...\n");
}

void achmed_reply(int fd, gamestate_t *state, void *data, int answer) {
	if (answer == 0)
	{
		if (state->gold >= 10)
		{
			state->gold -= 10;
			draw_statusbar(fd, state);
			
			kvs_list_req_t req;
			req.req.callid = RPC_CALL_KVS_LIST;
			req.kvstore = "librarian";
			rpcresult_t *res = rpccall(state->rpcfd, (rpcrequest_t*)&req);
			
			const char* buf = ((kvs_list_result_t*)res)->keys;
			show_multiline_text(fd,buf,achmed.npcdesc);
			
		}
		else show_multiline_text(fd, achmed_nogold, achmed.npcdesc);
	}
	else if (answer == 1) show_multiline_text(fd, achmed_no, achmed.npcdesc);
	else show_multiline_text(fd, achmed_honesty, achmed.npcdesc);
}


const mcdialog_t achmed_dialog = {
	.text = {
		"I'll buy such a certificate.",
		"No thanks, maybe later.",
		"How do I know you are honest?"
	},
	.callback = &achmed_reply
};

void handle_achmed(int fd, gamestate_t *state) {
	show_multiline_text(fd, achmed_greeting, achmed.npcdesc);
	start_mcinput(fd,state,&achmed_dialog,achmed_question,achmed.npcdesc,NULL);
}

npcstruct_t achmed = {
	.label = 'A',
	.npcdesc = "Honest Achmed",
	.npcfunc = &handle_achmed,
	.next = 0
};
