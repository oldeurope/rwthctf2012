#include "npc_vendor.h"

#include <stdlib.h>
#include <stdio.h>
#include "game_engine.h"
#include "libmudcom/map.h"

const char vendor_text[] = "Welcome to my store, are you interested in some seafood? I just received some fresh herrings. The red ones are really delicious!";
const char vendor_question[] = "So, what do you want?";
const char vendor_trouts[] = "We're out of trouts right now. I had to slap my apprentice a couple of times. You can try again tomorrow.";
const char vendor_amount[] = "How many do you want? They are 2 gold each.";
const char vendor_fish[] = "Here is your fish. Thank you for shopping here, have a nice day!";

void vendor_buy_fish(int fd, gamestate_t* state, void *data, const char* input) {
	int n = atoi(input);
	state->gold -= 2*n;
	draw_statusbar(fd,state);
	show_multiline_text(fd,vendor_fish,vendor.npcdesc);
}

void vendor_reply(int fd, gamestate_t* state, void *data, int answer) {
	if (answer == 0)
	{
		start_textinput(fd,state,vendor_amount,vendor.npcdesc,&vendor_buy_fish,NULL);
	}
	else if (answer == 1) 
	{
		char buf[128];
		long int p = (long int)state;
		int s = sizeof(long int) / 2;
		snprintf(buf,sizeof(buf),"The next greengrocery is 0x%x'th street number 0x%x",(unsigned short)((p<<s)>>s),(unsigned short)(p>>s));
		show_multiline_text(fd,buf,vendor.npcdesc);
	}
	else show_multiline_text(fd,vendor_trouts,vendor.npcdesc);
}

const mcdialog_t vendor_dialog = {
	.text = {
		"I'd like to buy some fish.",
		"Actually, I was looking for vegetables...",
		"No thanks. I was looking for trouts."
	},
	.callback = &vendor_reply
};

void handle_vendor(int fd, gamestate_t *state) {
	show_multiline_text(fd,vendor_text,vendor.npcdesc);
	start_mcinput(fd,state,&vendor_dialog,vendor_question,vendor.npcdesc,NULL);
}

npcstruct_t vendor = {
	.label = 'V',
	.npcdesc = "Loquacious Vendor",
	.npcfunc = &handle_vendor,
	.next = 0
};
