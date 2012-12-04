
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <assert.h>
#include "../game_engine.h"
#include "map.h"
#include "telnet.h"

const int initial_board[] = {
	_W, _W, _W, _W, _W, _W, _W, _W, _W, _W,	_W, _W, _W, _W, _W, _W, _W, _W, _W, _W,	_W, _W, _W, _W, _W, _W, _W, _W, _W, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _W,
	_W, _E, _E, _E, _E, _W, _W, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _W, _W, _W, _W,	_W, _W, _W, _W, _W, _W, _W, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _W, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _W, _E,	_E, _E, _W, _E, _E, _E, _E, _E, _E, _W,
	_W, _W, _W, _W, _W, _W, _W, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _W, _E, _E, _E, _E, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _W, _W, _W, _W, _W, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _W, _W, _E, _E,	_E, _E, _W, _E, _E, _E, _W, _E, _E, _W,
	_W, _E, _E, _E, _E, _W, _W, _W, _W, _W,	_W, _W, _E, _E, _E, _E, _E, _W, _E, _E,	_E, _E, _W, _E, _E, _E, _W, _E, _E, _W,
	_W, _E, _E, _E, _E, _W, _E, _E, _E, _E,	_E, _W, _E, _E, _E, _E, _E, _W, _E, _E,	_E, _E, _W, _E, _E, _E, _W, _E, _E, _W,
	_W, _E, _E, _E, _E, _W, _E, _E, _E, _E,	_E, _W, _E, _E, _E, _E, _E, _W, _E, _E,	_E, _E, _W, _E, _E, _E, _W, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _W, _E, _E, _E, _E, _E, _W, _E, _E,	_E, _E, _W, _E, _E, _E, _W, _W, _W, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _W, _E, _E, _E, _E, _E, _W, _E, _E,	_E, _E, _W, _E, _E, _E, _E, _E, _E, _W,
	_W, _W, _W, _W, _W, _W, _W, _W, _W, _W,	_W, _W, _W, _W, _W, _E, _E, _W, _E, _E,	_E, _E, _W, _E, _E, _E, _E, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _W, _E, _E, _W, _E, _E, _W, _W, _W,	_W, _W, _W, _E, _E, _E, _E, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _W, _E, _E, _W, _E, _E, _E, _E, _E,	_E, _E, _W, _W, _E, _E, _E, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _W, _E, _E, _W, _E, _E, _E, _E, _E,	_E, _E, _E, _W, _W, _W, _E, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _W,
	_W, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _E,	_E, _E, _E, _E, _E, _E, _E, _E, _E, _W,
	_W, _W, _W, _W, _W, _W, _W, _W, _W, _W,	_W, _W, _W, _W, _W, _W, _W, _W, _W, _W,	_W, _W, _W, _W, _W, _W, _W, _W, _W, _W,
};


void initialize_board(int *board) {
	memcpy(board,initial_board,sizeof(initial_board));
}

void draw_statusbar(int fd, const gamestate_t *state) {
	telnet_set_cursor(fd,0,0);
	char buf[128];
	snprintf(buf,sizeof(buf),"\x1B[1;34mrwthCTF\x1B[0m interactive Dungeons\t\t\x1B[33m%i gp\x1B[0m ",state->gold);
	telnet_send_wrapper(fd,buf,strlen(buf));

}

void game_home_cursor(int fd) {
	game_home_cursor_ext(fd,0);
}

void game_home_cursor_ext(int fd, int offset) {
	telnet_set_cursor(fd, 0, STATUSBAR_OFF+HEIGHT+offset);
}

char* write_color_code(char *buf, const char* color_num) {
	*buf++ = '\x1B';
	*buf++ = '[';
	*buf++ = color_num[0];
	if (color_num[1] != 0)
		*buf++ = color_num[1];
	*buf++ = 'm';
	return buf;
}

char* write_color_code_bright(char *buf, const char *color_num) {
	*buf++ = '\x1B';
	*buf++ = '[';
	*buf++ = '1';
	*buf++ = ';';
	*buf++ = color_num[0];
	if (color_num[1] != 0)
		*buf++ = color_num[1];
	*buf++ = 'm';
	return buf;
}

void draw_map_complete(int fd, const gamestate_t *state) {
	// send terminal clearing characte
	telnet_clear_screen(fd);
	telnet_set_cursor(fd,0,STATUSBAR_OFF);
	char linebuf[16*WIDTH+3];
	for (unsigned int y = 0; y < HEIGHT; ++y) {
		memset(linebuf,0,sizeof(linebuf));
		char *wrptr = linebuf;
		for (unsigned int x = 0; x < WIDTH; ++x) {
			if (x == state->posX && y == state->posY) {
				wrptr = write_color_code(wrptr,"31");
				*wrptr++ = '*';
				wrptr = write_color_code(wrptr,"0");
			}
			else if (state->board[y*WIDTH+x] == _W) {
				wrptr = write_color_code_bright(wrptr,"30");
				*wrptr++ = '#';
				wrptr = write_color_code(wrptr,"0");
			}
			else if (state->board[y*WIDTH+x] == _E) {
				*wrptr++ = ' ';
			}
			else {
				wrptr = write_color_code_bright(wrptr,"36");
				*wrptr++ = (char)state->board[y*WIDTH+x];
				wrptr = write_color_code(wrptr,"0");
			}
		}
		*wrptr++ = '\r';
		*wrptr++ = '\n';
		telnet_send_wrapper(fd, linebuf, strlen(linebuf));
	}
}

void game_clear_textarea(int fd) {
	game_home_cursor(fd);
	telnet_erase_down(fd);
}

void update_map_at(int fd, const gamestate_t *state, int x, int y) {
	// set cursor to update position
	telnet_set_cursor(fd,x,y+STATUSBAR_OFF);
	char outbuf[16];
	char *wrptr = outbuf;

	if (x == state->posX && y == state->posY) {
		wrptr = write_color_code(wrptr,"31");
		*wrptr++ = '*';
		wrptr = write_color_code(wrptr,"0");
	}
	else if (state->board[y*WIDTH+x] == _W) {
		wrptr = write_color_code_bright(wrptr,"30");
		*wrptr++ = '#';
		wrptr = write_color_code(wrptr,"0");
	}
	else if (state->board[y*WIDTH+x] == _E) {
		*wrptr++ = ' ';
	}
	else {
		wrptr = write_color_code_bright(wrptr,"36");
		*wrptr++ = (char)state->board[y*WIDTH+x];
		wrptr = write_color_code(wrptr,"0");
	}
	*wrptr=0;

	telnet_send_wrapper(fd,outbuf,strlen(outbuf));
}

int player_can_move(const gamestate_t *state, int dir, int *newX, int *newY, npcstruct_t **npc) {
	int toX = 0, toY = 0;
	if (dir == DIR_UP) {
		if (state->posY == 0)
			return 0;
		toY = state->posY - 1;
		toX = state->posX;
	}
	else if (dir == DIR_DOWN) {
		if (state->posY >= HEIGHT-1)
			return 0;
		toY = state->posY + 1;
		toX = state->posX;
	}
	else if (dir == DIR_LEFT) {
		if (state->posX == 0)
			return 0;
		toX = state->posX - 1;
		toY = state->posY;
	}
	else if (dir == DIR_RIGHT) {
		if (state->posX >= WIDTH-1)
			return 0;
		toX = state->posX + 1;
		toY = state->posY;
	}
//	printf("checking whether move is possible to %i,%i sign %x\n",toX,toY,state->board[toY*WIDTH+toX]);
	if (state->board[toY*WIDTH+toX] == _E) {
		*newX = toX;
		*newY = toY;
		return 1;
	}
	// check for npc
	*npc = NULL;
	npcstruct_t *npc_iter = state->npcs;
	while (npc_iter) {
//		printf("checking content %i against npc label %i\n", state->board[toY*WIDTH+toX], npc_iter->label);
		if ((unsigned char)state->board[toY*WIDTH+toX] == npc_iter->label) {
			*npc = npc_iter;
			return 0;
		}
		npc_iter = npc_iter->next;
	}
	return 0;
}

void draw_text(int fd, const char *text) {
	game_home_cursor(fd);
	telnet_erase_current_line(fd);
	telnet_send_wrapper(fd,text,strlen(text));
	game_home_cursor(fd);
}


static const char continue_msg[] = "(press any key to continue)";
void show_multiline_text(int fd, const char *text, const char *name) {
	// linenize the string as good as we can.
	char buffer[1024];
	snprintf(buffer,sizeof(buffer),"\x1B[1;36m%s:\x1B[0m %s",name,text);
	char *current = buffer;
	while (strlen(current) > LINE_LENGTH && current < buffer + sizeof(buffer)) {
		char *lookahead = current + LINE_LENGTH;
		// rsearch for spaces
		while (lookahead > current && *lookahead != ' ')
			--lookahead;
		assert(lookahead != current); // in this case words are too long - bad
		*lookahead = '\n';
		current = lookahead;
	}

	char *line = strtok(buffer,"\n");
	do {
		game_home_cursor(fd);
		telnet_erase_current_line(fd);
		telnet_send_wrapper(fd,line,strlen(line));
		line = strtok(NULL, "\n");

		game_home_cursor_ext(fd,1);
		telnet_erase_current_line(fd);
		if (line) telnet_send_wrapper(fd,line,strlen(line));
		line = strtok(NULL, "\n");

		game_home_cursor_ext(fd,2);
		telnet_send_wrapper(fd,continue_msg,sizeof(continue_msg)-1);
		// wait for continuation
		unsigned char c;
		telnet_readchar(fd,&c,0);
	} while (line);
	// erase continuation msg
	game_clear_textarea(fd);
}

void draw_mc_dialog(int fd, const mcdialog_t *dialog, int active_answer) {
	game_home_cursor_ext(fd,1);
	telnet_erase_down(fd);
	char linebuf[128];
	for (int i = 0; i < MC_NUM_OPTIONS; ++i) {
		if (i == active_answer) {
			snprintf(linebuf,sizeof(linebuf),"\x1B[1;31m-> %s\x1B[0m\r\n",dialog->text[i]);
		} else {
			snprintf(linebuf,sizeof(linebuf),"\x1B[31m-> %s\x1B[0m\r\n",dialog->text[i]);
		}
		telnet_send_wrapper(fd,linebuf,strlen(linebuf));
	}

}
