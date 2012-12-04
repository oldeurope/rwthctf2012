#include "Parser.h"


void Parser::tokenize(const string& str, vector<string>& tokens)
{
	istringstream iss(str);
	copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter<vector<string> >(tokens));
}

Parser::Parser(AbstractGame* game) {
    gameReference = game;
}

void Parser::run() {
    string line = "";   // will hold the full input line

	do {
		cout << "> ";

		getline(cin,line);

		if (line == "quit" || line == "exit" || line == "")
			break;

		if (!handleInput(line)) {
			cout << "I dont understand the command." << endl;
		}
	} while (1);

}

bool Parser::handleInput(string line) 
{
 
	vector<string> tokens;

	tokenize(line,tokens);

	vector<AbstractCommand*> executors = gameReference->getCommandExecutors();

	for (unsigned int i = 0; i < tokens.size(); ++i) 
	{
        string cmd = tokens[i];
		
		for (unsigned int j= 0 ; j<  executors.size();++j)
		{
			vector<string> tmp = executors[j]->getRecognizedCommands();

			for (unsigned int k = 0; k < tmp.size(); ++k)
			{
				 if (cmd == tmp[k])
				 {
					 if (executors[j]->WantsCleanedLine)
					 {
						 line = cleanseLine(line);
						 cout << endl;
						 Command tmpCMD(cmd,line.length() > cmd.length() +2 ? line.substr(cmd.length()+2) : "");
						 gameReference->processCommand(tmpCMD);
					 }
					 else
					 {
						 cout << endl;
						 Command tmpCMD(cmd,line.length() > cmd.length() +2 ? line.substr(cmd.length()+1) : "");
						 gameReference->processCommand(tmpCMD);					 
					 }
					 return true;
				 }
			}
		}
		
    }
    //no executor for the supplied command

    return false;
}

string Parser::cleanseLine(string line) {

	string result = "";
	vector<string> tokens;
	tokenize(line,tokens);

	for(unsigned int i = 0; i < tokens.size();++i)
	{
		if (tokens[i] == "at"||
			tokens[i] == "the" ||
			tokens[i] == "a" ||
			tokens[i] == "an")
			continue;

		result += " " + tokens[i];
	}
    return result;
}


Parser::~Parser(void)
{
}
