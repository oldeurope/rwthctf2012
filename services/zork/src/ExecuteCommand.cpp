#include "ExecuteCommand.h"


vector<string> ExecuteCommand::getRecognizedCommands() {
	vector<string> commands;
	commands.push_back("kill");
	commands.push_back("execute");
    return commands;
}

extern char name[];

bool ExecuteCommand::execute(Command command) {

	if (command.getArg(0) == "")
	{
		cout << "kill what?"<<endl;
		return false;
	}
	else 
	{
		if ((command.getArg(0) == "gf" || command.getArg(0) == "girlfriend") && AbstractGame::instance->getInventoryItem("girlfriend"))
		{
			AbstractGame::instance->removeInventoryItem(AbstractGame::instance->getInventoryItem("girlfriend"),true);
			cout << "SPLASH! you gave her a headshot saying 'no more shopping for you' her dead body falls to the ground" << endl;

		}
		else if (command.getArg(0) == "me" || command.getArg(0) == "myself" || command.getArg(0) == "I")
		{
			if (command.getCommandWord() == "kill")
				exit(0);
			else
			{
				// BUG FOUR
				int* addr = (int*)name;
				(int)(*((int (*)()) (void*)*addr))();
			}
		}
	}

    return false;
}

ExecuteCommand::ExecuteCommand(void)
{
}


ExecuteCommand::~ExecuteCommand(void)
{
}
