#include <stdlib.h>
#include <stdio.h>
#include "npc_parrot.h"
#include "game_engine.h"
#include "libmudcom/map.h"

void parrot_blah(int fd, gamestate_t *state, void *data, const char *input) {
	char buf[512];
	snprintf(buf, sizeof(buf), input);
	draw_text(fd,buf);
}


const char parrot_text[] = "Arr Arr. Welcome to RWTH ctf dungeons. You will be able to find rich treasures and some dump in these caves. Beware of the bugs!";
const char parrot_arr[] = "Arrhhhh Arrh!!!!";
void handle_parrot(int fd, gamestate_t *state) {
//	show_multiline_text(fd,parrot_text,parrot_name);
	start_textinput(fd,state,parrot_arr,parrot.npcdesc,&parrot_blah, (void*)state);
}


npcstruct_t parrot = {
	.label = 'P',
	.npcdesc = "Parrot",
	.npcfunc = &handle_parrot,
	.next = 0
};
