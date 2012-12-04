#pragma once

#include "../AbstractGame.h"
#include "AbstractCommand.h"
#include "../Command.h"
#include "../model/Direction.h"

#include <string>
#include <vector>

#include <iostream>
using namespace std;

class NavigateCommand : AbstractCommand
{
public:
    virtual vector<string> getRecognizedCommands();
    
	virtual bool execute(Command command);

	NavigateCommand(void);
	virtual ~NavigateCommand(void);
};
