#include "ExamineCommand.h"

vector<string> ExamineCommand::getRecognizedCommands()
{
	vector<string> commands;
	commands.push_back("examine");
	commands.push_back("look");
	return commands;
}

bool ExamineCommand::execute(Command command)
{
	Item* item = AbstractGame::instance->getCurrentRoom()->getItem(command.getArgsstring());
    if (item == NULL) {
        item = AbstractGame::instance->getInventoryItem(command.getArgsstring());
    }
    if (item != NULL) {
		if (item->getType() == ItemType::Literature) {
            cout << "It reads: \n\n" << item->examine() << endl;
        } else {
            cout << item->examine() << endl;
        }
    } else {
        //not examining an item, so let's check out the surroundings
        AbstractGame::instance->printCurrentRoom(true);
    }
    return false;
}


ExamineCommand::ExamineCommand(void)
{
}

ExamineCommand::~ExamineCommand(void)
{
}
