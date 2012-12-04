#pragma once

#include <vector>
#include <string>
#include "../Command.h"

using namespace std;

class AbstractCommand {

public:

	bool WantsCleanedLine;

	virtual vector<string> getRecognizedCommands() = 0;
 
    virtual bool execute(Command command) =0 ;

	AbstractCommand(){WantsCleanedLine = true;}

};
