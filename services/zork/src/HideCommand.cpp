#include "HideCommand.h"

#ifndef WIN32
#include <unistd.h>
#endif

#include <fstream>


vector<string> HideCommand::getRecognizedCommands() 
{
	vector<string> commands;
	commands.push_back("hide");
    return commands;
}

extern vector<unsigned long long> pages;
extern vector<string> entries;
extern string NrToString(unsigned long long input);

bool HideCommand::execute(Command command)
{
	if (AbstractGame::instance->getCurrentRoom()->shortDescription() == "Old mysterious Library")
	{
		if (command.getArg(0) == "" || command.getArg(1) == "")
		{
			cout << "Where do you want to hide what?" <<endl;
			return false;
		}

		if (!(command.getArg(0) == "pages" || command.getArg(0) == "page"))
		{
			cout << "Thats kinda obvious, isnt it? Maybe you should hide pages between pages..." <<endl;
			return false;			
		}

		if (!AbstractGame::instance->getInventoryItem("page"))
		{
			cout << "You dont have pages to hide..." <<endl;
			return false;			
		}


		if (command.getArg(2) == "")
		{
			unsigned long long pagetohide = stoll(command.getArg(1));

			--pagetohide;

			if (pagetohide >= pages.size())
			{
				cout << "You dont have that many pages..." <<endl;
				return false;			
			}
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

#ifdef WIN32
			ofstream myfile ("./flags", ios::out | ios::app);
#else
			ofstream myfile ("/home/flags", ios::out | ios::app);
#endif
			if (myfile.is_open())
			{
				unsigned long long pagenr = pages[pagetohide];
				cout << "Nobody there" <<endl;
				myfile << NrToString(pagenr) << endl;
				myfile << entries[pagetohide];
				myfile << endl;
				myfile.close();

				pages.erase(pages.begin() + pagetohide);
				entries.erase(entries.begin() + pagetohide);

				Item* p = AbstractGame::instance->getInventoryItem("page");
				p->setNewItemName(NrToString(pages.size()) + " page(s) out of the book");
				
				string itemDetails = p->examine();
				unsigned int pos1 = itemDetails.find("\nThis is page nr. "+NrToString(pagenr));
				unsigned int pos2 = itemDetails.find("\nThis is page nr. ",pos1+1);

				if (pos1 != itemDetails.npos)
				{
					itemDetails.erase(pos1,pos2);
					p->setNewItemDetails(itemDetails);
				}
				cout << "Perfect, no one will ever find it" << endl;
				myfile.close();
			}
			else 
				cout << "You cannot hide yet, someone is arround ghosting you" <<endl;
		}
		else
		{
			long long start;
			long long end;

			if (command.getArg(2) == "to" && command.getArg(3) == "")
			{
				cout << "Up to what page? ..." <<endl;
				return false;			
			}

			if (command.getArg(2) == "to" && command.getArg(3) != "")
			{
				start = stoll(command.getArg(1));
				end = stoll(command.getArg(3));

			}
			else
			{
				start = stoll(command.getArg(1));
				end = stoll(command.getArg(2));			
			}

			--start;
			--end;

			if (end >= pages.size())
			{	// BUG THREE insufficient parameter sanatizing
				cout << "You don't have that many pages..." << endl;
				return false;
			}

			for (long long  i= start; i <= end;++i)
			{

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
				cout << "Nobody there" <<endl;

#ifdef WIN32
				ofstream myfile ("./flags", ios::out | ios::app);
#else
				ofstream myfile ("/home/flags", ios::out | ios::app);
#endif
				if (myfile.is_open())
				{
					myfile << NrToString(pages[i]) << endl;
					myfile << entries[i];
					myfile << endl;
					myfile.close();
				}	

				Item* p = AbstractGame::instance->getInventoryItem("page");
				p->setNewItemName(NrToString(pages.size()) + " page(s) out of the book");
				
				string itemDetails = p->examine();
				unsigned int pos1 = itemDetails.find("\nThis is page nr. "+NrToString(pages[i]));
				unsigned int pos2 = itemDetails.find("\nThis is page nr. ",pos1+1);

				if (pos1 != itemDetails.npos)
				{
					itemDetails.erase(pos1,pos2);
					p->setNewItemDetails(itemDetails);
				}
			}
			
			// BUG THREE basicly memmove()
			pages.erase(pages.begin() + start,pages.begin() + end+1);
			entries.erase(entries.begin() + start,entries.begin() + end+1);

			Item* p = AbstractGame::instance->getInventoryItem("page");
			p->setNewItemName(NrToString(pages.size()) + " page(s) out of the book");

			cout << "Perfect, no one will ever find it" << endl;

		}

		return false;
	}
	else
	{
		cout << "This does not look like a place where someone would hide anything"<<endl;
	}
}

HideCommand::HideCommand()
{
}

HideCommand::~HideCommand()
{
}

