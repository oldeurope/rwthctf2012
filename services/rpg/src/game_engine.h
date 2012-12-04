#ifndef ENGINE_H
#define ENGINE_H
#include "libmudcom/common.h"

void connectionMain(int fd);

// gamestate mangement functions
int handle_gamestate_movement(int fd, gamestate_t *state);
int handle_gamestate_enter_text(int fd, gamestate_t *state, int linelength);
int handle_gamestate_mc_input(int fd, gamestate_t *state);


void init_gamestate(gamestate_t *state);
void runGame(int fd, int rpcfd, int linelength);
void register_npc(gamestate_t* state, npcstruct_t*);
void start_textinput(int fd, gamestate_t*, const char *desc, const char *name, textinputcallback_t callback, void *data);
void start_mcinput(int fd, gamestate_t*, const mcdialog_t *dialog, const char *desc, const char *name, void *data);

void debugStore(int fd, gamestate_t* state);

#endif
