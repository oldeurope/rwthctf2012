#include "FormatCommand.h"


vector<string> FormatCommand::getRecognizedCommands() {
	vector<string> commands;
	commands.push_back("format");
    return commands;
}

bool hadRevenge = false;

bool FormatCommand::execute(Command command) {
	if (AbstractGame::instance->getCurrentRoom()->shortDescription() == "Your Worst Enemys House" )
	{
		if (command.getArg(0) == "")
		{
			cout << "Required parameter missing -"<<endl;
			return false;
		}
		else if (command.getArg(0).length() == 2 && command.getArg(0)[1] == ':')
		{
			cout << "The screen says : " << endl;


			cout << "WARNING, ALL DATA ON NON-REMOVABLE DISK" << endl
				 << "DRIVE " << command.getArg(0) << " WILL BE LOST!"<<endl
				 << "Proceed with Format (Y/N)?";
			string y;
			getline(cin,y);

			if(y == "Y" || y == "y")
			{
				cout << "Checking existing disk format." << endl
					 << "Recording current bad clusters" << endl
					 << "Verifying 546,047.46M" <<endl
					 << "Format complete." << endl
					 << "Writing out file allocation table"<< endl
					 << "Complete."<<endl
					 << "Calculating free space (this may take several minutes)..." << endl
					 << "Complete."<<endl;

				cout << "HAHAHAA you got your Revenge" << endl;
				hadRevenge  = true;
			}

			cout << endl;
		
		}
		else 
		{
			cout << "'";
			// BUG FIVE
			printf(command.getArg(0).c_str());
			cout << "' is not recognized as an internal or external command, operable program or batch file.";
		}
	}
	else if(AbstractGame::instance->getCurrentRoom()->shortDescription() != "Your Room")
	{
		cout << "Really? Format your pc? just because its windows installed on it? Leave windows alone!"<<endl;
	}
	else
	{
		cout << "There need to be a computer arround to test this";
	}
    return false;
}

FormatCommand::FormatCommand(void)
{
}


FormatCommand::~FormatCommand(void)
{
}
