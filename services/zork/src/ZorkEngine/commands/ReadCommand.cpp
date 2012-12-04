#include "ReadCommand.h"

vector<string> ReadCommand::getRecognizedCommands() {
	vector<string> commands;
	commands.push_back("read");
	commands.push_back("r");
    return commands;
}

bool ReadCommand::execute(Command command) {
	Item* item = AbstractGame::instance->getCurrentRoom()->getItem(command.getArgsstring());
    if (item == NULL) {
        item = AbstractGame::instance->getInventoryItem(command.getArgsstring());
    }

	if (item != NULL) {
		if (item->getType() == ItemType::Literature) {
            cout << "\n\"" << item->examine() << "\"" <<endl;
            cout << endl;
			//AbstractGame::instance->addToInventory(item);
        } else {
            cout << "Nothing happened" << endl;
        }
    } else if (command.getArgsstring().empty()) {
        cout << "Read what?" << endl;
    } else {
        cout << "There's nothing to read here"<< endl;
    }
    return false;
}



ReadCommand::ReadCommand(void)
{
}

ReadCommand::~ReadCommand(void)
{
}
