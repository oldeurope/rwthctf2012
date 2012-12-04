#include "NavigateCommand.h"


vector<string> NavigateCommand::getRecognizedCommands() {
	vector<string> commands;
	commands.push_back("north");
	commands.push_back("east");
	commands.push_back("south");
	commands.push_back("west");
	commands.push_back("southwest");
	commands.push_back("southeast");
	commands.push_back("northwest");
	commands.push_back("northeast");

	commands.push_back("n");
	commands.push_back("e");
	commands.push_back("s");
	commands.push_back("w");
	commands.push_back("sw");
	commands.push_back("se");
	commands.push_back("nw");
	commands.push_back("ne");


    return commands;
}

bool NavigateCommand::execute(Command command) {
    Direction direction;
    if (command.getCommandWord() == "n" || command.getCommandWord() == "north") {
		direction = Direction::north;
    } else if (command.getCommandWord() == "e" || command.getCommandWord() == "east") {
		direction = Direction::east;
    } else if (command.getCommandWord() == "s" || command.getCommandWord() == "south") {
		direction = Direction::south;
    } else if (command.getCommandWord() == "w" || command.getCommandWord() == "west") {
		direction = Direction::west;
    } else if (command.getCommandWord() == "se" || command.getCommandWord() == "southeast") {
		direction = Direction::southeast;
    } else if (command.getCommandWord() == "sw" || command.getCommandWord() == "southwest") {
		direction = Direction::southwest;
    } else if (command.getCommandWord() == "ne" || command.getCommandWord() == "northeast") {
		direction = Direction::northeast;
    } else if (command.getCommandWord() == "nw" || command.getCommandWord() == "northwest") {
		direction = Direction::northwest;
    } else {
		direction = Direction::none;
    }
    AbstractGame::instance->goInDirection(direction);
    return false;
}



NavigateCommand::NavigateCommand(void)
{
}

NavigateCommand::~NavigateCommand(void)
{
}
