#include "TakeCommand.h"

vector<string> TakeCommand::getRecognizedCommands() 
{
	vector<string> commands;
	commands.push_back("take");
	commands.push_back("t");
    return commands;
}

bool TakeCommand::execute(Command command)
{
    Item* item = AbstractGame::instance->getCurrentRoom()->getItem(command.getArgsstring());
	if (item == NULL) 
	{
        item = AbstractGame::instance->getInventoryItem(command.getArgsstring());
    }

    if (item != NULL)
	{
        if (item->canInventory()) 
		{
            AbstractGame::instance->addToInventory(item);
        } 
		else 
		{
            cout << "You don't need to take this item."<< endl;
        }
    } 
	else 
	{
        cout << "There's nothing to take with that name!" << endl;
    }
    return false;
}

TakeCommand::TakeCommand()
{
}

TakeCommand::~TakeCommand()
{
}

