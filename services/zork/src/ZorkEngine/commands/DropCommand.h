#pragma once

#include "AbstractCommand.h"
#include "../AbstractGame.h"
#include "../Command.h"
#include "../model/Item.h"

#include <string>
#include <vector>

#include <iostream>
using namespace std;

class DropCommand : AbstractCommand {

public:

    virtual vector<string> getRecognizedCommands();
    
    virtual bool execute(Command command);

	DropCommand();
	virtual ~DropCommand();
};