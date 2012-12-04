#include "InventoryCommand.h"


vector<string> InventoryCommand::getRecognizedCommands() {
	vector<string> commands;
	commands.push_back("inventory");
	commands.push_back("i");
    return commands;
}

bool InventoryCommand::execute(Command command) {
	AbstractGame::instance->printInverntory();
    return false;
}


InventoryCommand::InventoryCommand(void)
{
}

InventoryCommand::~InventoryCommand(void)
{
}
