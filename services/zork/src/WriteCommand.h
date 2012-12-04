#pragma once

#include "ZorkEngine/AbstractGame.h"
#include "ZorkEngine/Command.h"

#include <string>
#include <vector>

#include <iostream>
using namespace std;


class WriteCommand : AbstractCommand
{
public:
    virtual vector<string> getRecognizedCommands();
    
	virtual bool execute(Command command);


	WriteCommand(void);
	virtual ~WriteCommand(void);
};
