#include "UseCommand.h"

vector<string> UseCommand::getRecognizedCommands() {
	vector<string> commands;
	commands.push_back("use");
	commands.push_back("apply");
	commands.push_back("give");
	commands.push_back("put");
	commands.push_back("place");
    return commands;
}

bool UseCommand::execute(Command command) {
 
	if (command.getArg(0) == "") {
        cout << "Huh?"<<endl;
        return false;
    }

    //find preposition
    int prepIdx = -1;


	for (unsigned int i = 0; i < command.getArgs().size(); ++i)
	{
		prepIdx++;
		if ( command.getArgs()[i] == "into" || 
			 command.getArgs()[i] == "in" || 
			 command.getArgs()[i] == "on" ||  
			 command.getArgs()[i] == "to")
			 break;
	}

    if (prepIdx <= 0) {
        if (command.getArgsstring().empty()) {
            cout << "Huh?"<< endl;
            return false;                
        } else {
            cout << "What do you want to use it on?" << endl;
            return false;
        }
    }

    string toUse = "";
    string usedOn = "";
    for (int s2=0;s2<prepIdx;s2++) {
        if (s2 != 0) {
         toUse += " ";
        }
        toUse += command.getArg(s2);
    }
    for (unsigned int s3=prepIdx+1;s3<command.getArgs().size();s3++) {
        if (s3 != prepIdx+1) {
         usedOn += " ";
        }
        usedOn += command.getArg(s3);
    }

    if (usedOn.empty()) {
        cout << "What do you want to use it on?" << endl;
        return false;
    }
	Item* item = AbstractGame::instance->getInventoryItem(toUse);
    Item* item2 = AbstractGame::instance->getInventoryItem(usedOn);
    
    if (item2 == NULL) {
        item2 = AbstractGame::instance->getCurrentRoom()->getItem(usedOn);
    }
    AbstractGame::instance->combineItems(item2, item);
    return false;
}




UseCommand::UseCommand(void)
{
}

UseCommand::~UseCommand(void)
{
}
