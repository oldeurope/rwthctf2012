#include "DropCommand.h"

vector<string> DropCommand::getRecognizedCommands() {
	vector<string> commands;
	commands.push_back("drop");
    return commands;
}
    
bool DropCommand::execute(Command command) {
	Item* item = AbstractGame::instance->getInventoryItem(command.getArgsstring());
    if (item != NULL) {
        bool wasRemoved = AbstractGame::instance->removeInventoryItem(item, false);
        if (wasRemoved) {
            cout << "Dropped the " << command.getArgsstring() << endl;
        } else {
            cout << "I would keep that if I were you" << endl;            
		}
    } else {
        cout << "Could not drop the item" << endl;
    }
    return false;
}


DropCommand::DropCommand(void)
{
}

DropCommand::~DropCommand(void)
{
}
