#ifndef COMMON_H
#define COMMON_H

struct gamestate;

#define MC_NUM_OPTIONS 3
#define TEXT_INPUT_LEN 256

typedef void (*npcfunc_t)(int, struct gamestate*);
typedef void (*textinputcallback_t)(int, struct gamestate*, void *data, const char *input);
typedef void (*mcinputcallback_t)(int, struct gamestate*, void *data, int answer);

struct npcstruct {
	unsigned char label;
	const char *npcdesc;
	npcfunc_t npcfunc;
	struct npcstruct *next;
};

struct mcdialog {
	const char *text[MC_NUM_OPTIONS];
	mcinputcallback_t callback;
};

struct gamestate {
	unsigned int posX;
	unsigned int posY;
	unsigned int gold;
	unsigned int state;
	int *board;
	// input methods - shared
	char *inputPtr;
	char textInputBuffer[TEXT_INPUT_LEN];
	void *callback_data;
	// free textinput
	textinputcallback_t textcallback;
	// mc input
	const struct mcdialog *dialog;
	int mcselection;
	// npc data
	struct npcstruct *npcs;
	int rpcfd;
};


typedef struct gamestate gamestate_t;
typedef struct npcstruct npcstruct_t;
typedef struct mcdialog mcdialog_t;

#define GAMESTATE_STOP 0
#define GAMESTATE_MOVEMENT 1
#define GAMESTATE_MC_INPUT 2
#define GAMESTATE_ENTER_TEXT 3

#endif
