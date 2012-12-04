#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace std;


class Command {
private:
    string commandWord;
    vector<string> args;
    string argsstring;

public:
    Command(string command, string args) {
		commandWord = command;
		argsstring = args;
		istringstream iss(args);
		copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter<vector<string> >(this->args));
    }

    string getCommandWord() {
        return commandWord;
    }

    string getArg(unsigned int index) {
		if (( args.size() > 0) && (index <= args.size() - 1)) {
            return args[index];
        } else {
            return "";
        }
    }

    vector<string> getArgs() {
        return args;
    }

    string getArgsstring() {
        return argsstring;
    }
};