#pragma once

#include "AbstractCommand.h"
#include "../AbstractGame.h"
#include "../Command.h"

#include <string>
#include <vector>

#include <iostream>
using namespace std;

class HelpCommand : AbstractCommand
{
public:
    virtual vector<string> getRecognizedCommands();
    
	virtual bool execute(Command command);

	HelpCommand(void);
	virtual ~HelpCommand(void);
};
