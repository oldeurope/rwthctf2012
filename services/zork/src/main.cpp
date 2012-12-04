#include <iostream>
#include "Example.h"

#ifndef WIN32
#include <unistd.h>
#endif

using namespace std;


/*
	intentionally introduced Bugs:

	Trivia: BUG ZERO

		- (Kategorie: fehlerhafte default Konfiguration) debugmode = true - by default. "search" without arguments lists the entire directory

	Low : BUG ONE
		
		- (Kategorie: logik) debugmode lässt sich durch Item Kombination wieder einschalten

	Medium: BUG TWO

		- (Kategorie: Memory Leak) give character name! overflow only character name -> overwrite flag delimter -> itterate readout flags;

	Hard: BUG THREE
		
		- (Kategorie: Code Execution) vector.erase bounds i.e. inventory items collect pages in old library by integer id! put them in vector! allow dropping them (erasing)! enjoy RCE!

	BUG FOUR:

		- execute myself direct code exec

	BUG FIVE:

		- format string on formatcommand

*/


int main(int argc,char* argv[])
{
	Example Game;
#if 0
	// Prevent bruteforcing small ints
	cout<< "[ ]Loading Game Engine : ";
	for (int i = 0; i < 40; ++i)
	{
		cout << ".";
#ifndef WIN32
		usleep(50);
#else
		_sleep(50);
#endif
	}
	cout << "\n[+] complete (That may be pretended loading :O )" << endl;
#endif

	Game.clearScreen();

	Game.init();
	Game.play();

	return NULL;
}


