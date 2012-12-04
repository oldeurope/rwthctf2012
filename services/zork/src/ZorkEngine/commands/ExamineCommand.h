#pragma once

#include "../AbstractGame.h"
#include "AbstractCommand.h"
#include "../Command.h"
#include "../model/Item.h"
#include "../model/ItemType.h"

#include <string>
#include <vector>

#include <iostream>
using namespace std;



class ExamineCommand : AbstractCommand {
public:
    virtual vector<string> getRecognizedCommands();
    
	virtual bool execute(Command command);

	ExamineCommand();
	virtual ~ExamineCommand();
};