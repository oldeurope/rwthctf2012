#pragma once

#include "AbstractCommand.h"

#include "../AbstractGame.h"
#include "../Command.h"
#include "../model/Item.h"
#include "../model/ItemType.h"
#include <string>
#include <vector>

#include <iostream>
using namespace std;


class ReadCommand : AbstractCommand
{
public:
    virtual vector<string> getRecognizedCommands();
    
	virtual bool execute(Command command);

	ReadCommand(void);
	virtual ~ReadCommand(void);
};
