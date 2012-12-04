#pragma once

#include "../AbstractGame.h"
#include "AbstractCommand.h"
#include "../Command.h"

#include <string>
#include <vector>

#include <iostream>
using namespace std;


class InventoryCommand : AbstractCommand
{
public:
    virtual vector<string> getRecognizedCommands();
    
	virtual bool execute(Command command);


	InventoryCommand(void);
	~InventoryCommand(void);
};
