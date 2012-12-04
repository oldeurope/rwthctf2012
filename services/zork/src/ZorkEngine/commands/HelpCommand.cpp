#include "HelpCommand.h"

vector<string> HelpCommand::getRecognizedCommands() {
	vector<string> commands;
	commands.push_back("help");
	commands.push_back("h");
    return commands;
}

bool HelpCommand::execute(Command command) {
	if (command.getArg(0) == "")
	{
		cout << "<help more> for more detailed help" << endl;
		AbstractGame::instance->printHelp(true);
	}
	else
	{
		AbstractGame::instance->printHelp(false);
	}
    return false;
}

HelpCommand::HelpCommand(void)
{
}

HelpCommand::~HelpCommand(void)
{
}
