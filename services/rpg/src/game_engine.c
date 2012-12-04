#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include "libmudcom/telnet.h"
#include "game_engine.h"
#include "libmudcom/map.h"
#include "rpc.h"

#include "npc_parrot.h"
#include "npc_nun.h"
#include "npc_achmed.h"
#include "npc_librarian.h"
#include "npc_vendor.h"

void init_gamestate(gamestate_t *state) {
	state->posX = 1;
	state->posY = 1;
	state->gold = 0;
	state->state = GAMESTATE_MOVEMENT;
	state->npcs = NULL;
	state->callback_data = NULL;
	state->textcallback = NULL;
	state->inputPtr = state->textInputBuffer;
	memset(state->textInputBuffer,0,TEXT_INPUT_LEN);
	state->dialog = NULL;
	state->mcselection = 0;
	state->board = malloc(sizeof(int)*WIDTH*HEIGHT);
	initialize_board(state->board);
}

#define MAX(a,b) (((a)>(b))?(a):(b))

void connectionMain(int fd) {
	srand( time(NULL) );

	int startGame = 0;
	int rpcfd = init_rpc();
	// AFTER SANDBOX initialization
	telnet_init(fd);
	char warning[] = "don't fuck up your telnet";
	telnet_send_wrapper(fd,warning,strlen(warning));
	// wait for window size negotiation
	do {
		int res;
		unsigned char c;
		res = telnet_readchar(fd,&c,1); // wait for negotiation
		if (res <= 0) {
			startGame = 0;
			break;
		}
		if (res == 2 && telnet_get_term_width() != -1) {
			// check window size
			if (telnet_get_term_width() < MAX(WIDTH,LINE_LENGTH) ||
				telnet_get_term_height() < HEIGHT + STATUSBAR_OFF + MAX_TEXTAREA_SIZE) {
				char buf[128];
				snprintf(buf,sizeof(buf),"Your terminal needs at least %i columns and %i rows in order to run this app!\n",MAX(WIDTH,LINE_LENGTH),HEIGHT+STATUSBAR_OFF+MAX_TEXTAREA_SIZE);
				telnet_send_wrapper(fd,buf,strlen(buf));
				break; // exit don't set startGame to 1 and do correct rpc cleanup
			}
			else {
				startGame = 1;
			}
		}

	} while (telnet_get_term_height() == -1);

	if (startGame)
		runGame(fd,rpcfd, telnet_get_term_width());

	close_sock_call_t closer;
	closer.req.callid = RPC_CALL_CLOSE_SOCK;
	closer.sockid = fd;
	rpccall(rpcfd,(rpcrequest_t*)&closer);

	rpcrequest_t req;
	req.callid = RPC_CALL_END_SERVICE;
	rpccall(rpcfd,&req);
	exit(0); // hope this also kills rpc thread
}

void runGame(int fd, int rpcfd, int linelength) {
	gamestate_t state;
	init_gamestate(&state);
	state.rpcfd = rpcfd;

	// register and place the npcs
	register_npc(&state,&parrot);
	register_npc(&state,&nun);
	register_npc(&state,&achmed);
	register_npc(&state,&librarian);
	register_npc(&state,&vendor);

	draw_map_complete(fd,&state);
	draw_statusbar(fd,&state);
	game_home_cursor(fd);
	telnet_terminal_hidecursor(fd);
	do {
		if (state.state == GAMESTATE_MOVEMENT) {
			if (handle_gamestate_movement(fd,&state) < 0) {
				return;
			}
		}
		else if (state.state == GAMESTATE_ENTER_TEXT) {
			if (handle_gamestate_enter_text(fd,&state, linelength) < 0) {
				return;
			}
		}
		else if (state.state == GAMESTATE_MC_INPUT) {
			if (handle_gamestate_mc_input(fd, &state) < 0) {
				return;
			}
		}
	} while (state.state != GAMESTATE_STOP);
	draw_text(fd,"Goodbye traveler!");
	game_home_cursor_ext(fd,1);
	telnet_terminal_showcursor(fd);
}


int handle_gamestate_movement(int fd, gamestate_t *state) {
	int res;
	unsigned char c;
	res = telnet_readchar(fd, &c, 0);
	if (res <= 0)
		return -1;

//			printf("telnet read char %i\n",c);
	// check for WASD
	int newX, newY, oldX, oldY;
	npcstruct_t *npc = NULL;

	oldX = state->posX;

	oldY = state->posY;

	res = 0;
	if (c == 'w')
		res = player_can_move(state,DIR_UP,&newX,&newY,&npc);
	else if (c == 'a') {
		res = player_can_move(state,DIR_LEFT,&newX,&newY,&npc);
	}
	else if (c == 's') {
		res = player_can_move(state,DIR_DOWN,&newX,&newY,&npc);
	}
	else if (c == 'd') {
		res = player_can_move(state,DIR_RIGHT,&newX,&newY,&npc);
	}
	else if (c == '\x03' || c == '\x04') {
		state->state = GAMESTATE_STOP;
		return 0;
	}
	if (res) {
		state->posX = newX;
		state->posY = newY;
		update_map_at(fd,state,oldX,oldY);
		update_map_at(fd,state,newX,newY);
		game_home_cursor(fd);
	}
	if (npc) {
		(npc->npcfunc)(fd,state);
	}
	return 0;
}

int handle_gamestate_enter_text(int fd, gamestate_t *state, int linelength) {
	// read one char and echo it back if its not \r or \n
	int res;
	unsigned char c;
	res = telnet_readchar(fd, &c, 0);
	if (res <= 0)
		return -1;
	if (c != '\r' && c != '\x7F') {
		// if space in buffer -> write and echo
		//if (state->inputPtr < state->textInputBuffer + TEXT_INPUT_LEN - 1) {
		// broken version: check only on terminal width
		if (state->inputPtr < state->textInputBuffer + linelength) {
			*state->inputPtr++ = c;
			telnet_send_wrapper(fd,&c,1);
		}
	}
	if (c == '\x7F') {
		if (state->inputPtr > state->textInputBuffer) {
			*--state->inputPtr = 0;
			// remove more bytes if they are prefixed with 0x80 because this indicates utf8 multibyte encoding
			while (state->inputPtr-1 > state->textInputBuffer && *(state->inputPtr-1) & 0x80) {
				*--state->inputPtr = 0;
			}
			telnet_terminal_backspace(fd);
		}
	}
	// if buffer is full or \n came
	//if (c == '\r' || state->inputPtr == state->textInputBuffer + TEXT_INPUT_LEN - 1) {
	// adapting broken version: check only on line length
	if (c == '\r' || state->inputPtr == state->textInputBuffer + linelength) {
		//*state->inputPtr = 0;
		// remove text input from terminal
		game_clear_textarea(fd);
		telnet_terminal_hidecursor(fd);
		state->state = GAMESTATE_MOVEMENT;
		state->inputPtr = state->textInputBuffer;
		textinputcallback_t callback = state->textcallback;
		void *cdata = state->callback_data;
		state->textcallback = NULL;
//		state->callback_data = NULL;

		if (callback) {
			(callback)(fd,state,cdata,state->textInputBuffer);
		}
		memset(state->textInputBuffer,0,TEXT_INPUT_LEN);
	}
	return 0;
}

int handle_gamestate_mc_input(int fd, gamestate_t *state) {
	int res;
	unsigned char c;
	res = telnet_readchar(fd, &c, 0);
	if (res <= 0)
		return -1;
	if (!state->dialog) {
		state->state = GAMESTATE_MOVEMENT;
		return 0;
	}

	// only check for answer change options which are w/s and enter
	if (c == 'w' && state->mcselection > 0) {
		state->mcselection--;
		draw_mc_dialog(fd,state->dialog,state->mcselection);
	}
	else if (c == 's' && state->mcselection < MC_NUM_OPTIONS-1) {
		state->mcselection++;
		draw_mc_dialog(fd,state->dialog,state->mcselection);
	}
	else if (c == '\r') {
		game_clear_textarea(fd);
		state->state = GAMESTATE_MOVEMENT;
		const mcdialog_t *dialog = state->dialog;
		void *cdata = state->callback_data;
		state->dialog = NULL;
		state->callback_data = NULL;

		if (dialog->callback) {
			(dialog->callback)(fd,state,cdata,state->mcselection);
		}
	}
	return 0;
}


void register_npc(gamestate_t* state, npcstruct_t *npc) {
	while (1) { // find position for npc
		int pos = rand() % (HEIGHT*WIDTH), ok = 1;
		if (state->board[pos] != _E) continue;
		if (state->posX + state->posY * WIDTH == pos) continue;
		for (int off = 0; off < 9; off++)
		{ // check surrounding squares for other npcs
			int check = pos + (off % 3)-1 + (off/3 - 1) * WIDTH;
			if ((check < 0) || (check >= WIDTH*HEIGHT)) continue;
			if ((state->board[check] != _W) && (state->board[check] != _E)) ok = 0;
			if (!ok) break;
		}
		if (!ok) continue;
		state->board[pos] = npc->label;
		break;
	}
	npcstruct_t *end = state->npcs;
	if (!end) {
		state->npcs = npc;
		npc->next = NULL;
		return;
	}
	while (end->next)
		end = end->next;
	end->next = npc;
	npc->next = NULL;
}


void start_textinput(int fd, gamestate_t *state, const char *desc, const char *name, textinputcallback_t callback, void *data) {
	char buf[256];
	snprintf(buf, sizeof(buf), "\x1B[1;36m%s:\x1B[0m %s", name, desc);
	draw_text(fd,buf);
	game_home_cursor_ext(fd,1);
	telnet_terminal_showcursor(fd);
	state->textcallback = callback;
	state->callback_data = data;
	state->state = GAMESTATE_ENTER_TEXT;
}


void start_mcinput(int fd, gamestate_t *state, const mcdialog_t *dialog, const char *desc, const char *name, void *data) {
	char buf[256];
	snprintf(buf, sizeof(buf), "\x1B[1;36m%s:\x1B[0m %s", name, desc);
	draw_text(fd,buf);
	draw_mc_dialog(fd, dialog,0);
	state->dialog = dialog;
	state->mcselection = 0;
	state->callback_data = data;
	state->state = GAMESTATE_MC_INPUT;

}

void debugStore(int fd, gamestate_t* state) {
	kvs_list_req_t req;
	req.req.callid = RPC_CALL_KVS_LIST;
	req.kvstore = "librarian";
	rpcresult_t *res = rpccall(state->rpcfd, (rpcrequest_t*)&req);
	char* buf = ((kvs_list_result_t*)res)->keys;
//	printf("%s", buf);
	telnet_send_wrapper(fd, buf, strlen(buf));
	free(buf);
	free(res);
}
