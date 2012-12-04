#include <stdlib.h>
#include "npc_nun.h"
#include "game_engine.h"
#include "libmudcom/map.h"

const char nun_text[] = "Good morning traveler. You seem hungry, hence I'll provide you with ten gold. May god protect you on your way.";
void handle_nun(int fd, gamestate_t *state) {
	show_multiline_text(fd,nun_text,nun.npcdesc);
	state->gold += 10;
	draw_statusbar(fd, state);
}


npcstruct_t nun = {
	.label = 'N',
	.npcdesc = "Gracious Nun",
	.npcfunc = &handle_nun,
	.next = 0
};
