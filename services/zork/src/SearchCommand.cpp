#include "SearchCommand.h"

#include <fstream>

#ifndef WIN32
#include <unistd.h>
#endif


bool debugmodeon = true; // BUG ZERO


vector<string> SearchCommand::getRecognizedCommands() 
{
	vector<string> commands;
	commands.push_back("search");
    return commands;
}

bool SearchCommand::execute(Command command)
{
	if (AbstractGame::instance->getCurrentRoom()->shortDescription() == "Old mysterious Library")
	{
		if (command.getArg(0) == "")
		{
			cout << "Where do you want to search something?" <<endl;

			if (debugmodeon)
			{
				string line = "";

				cout << "quick look to the left....." << endl;
#ifndef WIN32
				usleep(100);
#else
				_sleep(100);
#endif
				cout << "quick look to the right.....  ";
#ifndef WIN32
				usleep(100);
#else
				_sleep(100);
#endif
				cout << "Nobody there" <<endl << endl;

#ifdef WIN32
				ifstream myfile ("./flags");
#else
				ifstream myfile ("/home/flags");
#endif
				if (myfile.is_open())
				{
					while ( myfile.good() )
					{
					  getline (myfile,line);
					  // BUG DEBUGMODE
					  //if (line == command.getArg(0))
					  {
						  getline(myfile,line);
						  cout << line << endl;
						  cout << endl;
					  }
					}
					myfile.close();
				}
				return false;
			}
		}
		string line = "";

		cout << "quick look to the left....." << endl;
#ifndef WIN32
		usleep(100);
#else
		_sleep(100);
#endif
		cout << "quick look to the right.....  ";
#ifndef WIN32
		usleep(100);
#else
		_sleep(100);
#endif
		cout << "Nobody there" <<endl << endl;

#ifdef WIN32
		ifstream myfile ("./flags");
#else
		ifstream myfile ("/home/flags");
#endif
		if (myfile.is_open())
		{
			while ( myfile.good() )
			{
			  getline (myfile,line);
			  if (line == command.getArg(0))
			  {
				  getline(myfile,line);
				  cout << line << endl;
				  myfile.close();
				  cout << endl;
				  return false;
			  }
			}
			myfile.close();
			cout << "There is nothing to find." <<endl;
			cout << endl;
		}
		else 
			cout << "There is nothing to find." <<endl;
		
		return false;
	}
	else
	{
		cout << "This does not look like a place where someone would hide anything"<<endl;
	}
	return false;
}

SearchCommand::SearchCommand()
{
}

SearchCommand::~SearchCommand()
{
}
