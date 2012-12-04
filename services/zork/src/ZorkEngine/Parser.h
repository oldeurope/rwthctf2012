#pragma once


#include "commands/AbstractCommand.h"
#include "AbstractGame.h"

#include <string>
#include <iostream>

using namespace std;

class AbstractGame;

class Parser {
private:

	AbstractGame* gameReference;

public:
    
	void tokenize(const string& str, vector<string>& tokens/*, const string& delimiters = " "*/);
	
	Parser(AbstractGame* game);
	~Parser();

    virtual void run();

private:
    bool handleInput(string line);

	string cleanseLine(string line);
};