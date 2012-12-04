#include "WriteCommand.h"


vector<unsigned long long> pages;
vector<string> entries;

char name[32] = "0123456789abcdefABCDEF";
char stddelimiter[] = " "; // BUG TWO SEE BOT

string NrToString(unsigned long long input)
{
	stringstream ss;
	string s;
	ss << input;
	ss >> s; // or s = ss.str();
	return s;
}

vector<string> WriteCommand::getRecognizedCommands() 
{
	vector<string> commands;
	commands.push_back("write-on");
	commands.push_back("write");
    return commands;
}

bool WriteCommand::execute(Command command)
{
	if (AbstractGame::instance->getInventoryItem("big old book"))
	{
		if (command.getArg(0) == "" || command.getArg(1) == "" ||command.getArg(2) == "")
		{
			cout << "On which page you want to write what?" <<endl;
			return false;
		}

		if (command.getArg(0) != "page")
		{
			cout << "You cannot write on that..." << endl;
			return false;
		}

		unsigned long long page;
		try{
			page = std::stoull(command.getArg(1));
		}
		catch(...)
		{
			cout << "Thats not a page number" << endl;
			return false;
		}
		string entry = "";

		if (!page)
		{
			cout << "You cannot write on that..." << endl;
			return false;
		}

		pages.push_back(page);

		for(unsigned int  i= 2; i < command.getArgs().size(); ++i)
		{
			entry += command.getArg(i);
			entry += stddelimiter; // BUG TWO SEE TOP
		}

		entries.push_back(entry);

		cout << "Crrrsccccccchht you ripped that page out of the book"<<endl;

		if(!AbstractGame::instance->getInventoryItem("page"))
		{
			vector<string> tmpnote;
			tmpnote.push_back("page");
			tmpnote.push_back("pages");
			Item* note = new Item(ItemType::Misc, true, false, NrToString(pages.size()) + " page(s) out of the book",
				tmpnote,
				"pages out of the big old book",
				"\nThis is page nr. " + command.getArg(1)+" of the big old book and it reads:\n\n\t\"" + entry+"\"");
			AbstractGame::instance->addToInventory(note);
		}
		else
		{
			Item* note = AbstractGame::instance->getInventoryItem("page");
			note->setNewItemDetails(note->examine() + "\nThis is page nr. " + command.getArg(1)+" of the big old book and it reads:\n\n\t\"" + entry+"\"");
			note->setNewItemName(NrToString(pages.size()) + " page(s) out of the book");
		}


		return false;
	}
	else
	{
		cout << "You need a book or something to write to"<<endl;
	}
	return false;
}

WriteCommand::WriteCommand()
{
	WantsCleanedLine = false;
	pages.assign(10,0xcdcdcdcd);
	pages.clear();
	entries.assign(10,"");
	entries.clear();
}

WriteCommand::~WriteCommand()
{
}