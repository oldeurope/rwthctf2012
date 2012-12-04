#pragma once

#include "ZorkEngine/AbstractGame.h"
#include "ZorkEngine/Command.h"

#include <string>
#include <vector>

#include <iostream>
using namespace std;


class SearchCommand : AbstractCommand
{
public:
    virtual vector<string> getRecognizedCommands();
    
	virtual bool execute(Command command);


	SearchCommand(void);
	virtual ~SearchCommand(void);
};
