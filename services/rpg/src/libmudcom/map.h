#ifndef MAP_H
#define MAP_H
#include "common.h"

#define _E 0 // EMPTY
#define _W 1 // WALL

#define STARTX 1
#define STARTY 1

#define WIDTH 30
#define HEIGHT 20

#define DIR_UP 1
#define DIR_LEFT 2
#define DIR_RIGHT 3
#define DIR_DOWN 4

#define STATUSBAR_OFF 1
#define LINE_LENGTH 60
#define MAX_TEXTAREA_SIZE 4

void initialize_board(int *board);
void draw_map_complete(int fd, const gamestate_t *state);
void update_map_at(int fd, const gamestate_t *state, int x, int y);
void draw_statusbar(int fd, const gamestate_t *state);
void game_home_cursor(int fd);
void game_home_cursor_ext(int fd, int offset);
void draw_text(int fd, const char *text);
void show_multiline_text(int fd, const char *text, const char *name);
void game_clear_textarea(int fd);
void draw_mc_dialog(int fd, const mcdialog_t *dialog, int active_answer);

int player_can_move(const gamestate_t *state, int dir, int *newX, int *newY, npcstruct_t **npc);

#endif
