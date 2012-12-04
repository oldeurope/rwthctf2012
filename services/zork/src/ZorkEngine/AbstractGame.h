#pragma once


#include "commands/AbstractCommand.h"
#include "commands/DropCommand.h"
#include "commands/ExamineCommand.h"
#include "commands/HelpCommand.h"
#include "commands/InventoryCommand.h"
#include "commands/NavigateCommand.h"
#include "commands/ReadCommand.h"
#include "commands/TakeCommand.h"
#include "commands/UseCommand.h"

#include "model/Item.h"
#include "model/Direction.h"
#include "model/Room.h"

#include "Parser.h"

#include <vector>
#include <string>

#include <iostream>


using namespace std;

class Parser;

class AbstractGame
{
protected:
	Parser* parser;
	Room* currentRoom;
	vector<AbstractCommand*> executors;
	vector<Item*> inventory;

public:
	static AbstractGame* instance; 
	
	virtual void destroy();
	virtual void init();

	virtual void createRooms() = 0;
	virtual void createItems() = 0;
	virtual void combineItems(Item* item, Item* inventoryItem) =  0;  


	vector<AbstractCommand*> getCommandExecutors();
	void addBaseCommands();
	void addCommand(AbstractCommand* command);
	virtual void addToInventory(Item* item);
	Item* getInventoryItem(string itemName);
	virtual bool removeInventoryItem(Item* item, bool permanent);
	virtual void play();
	virtual void printWelcome();
	virtual bool processCommand(Command command);
	virtual void printHelp(bool SupresSyns = false);
	virtual void clearScreen();
	virtual void printInverntory();

	virtual void goInDirection(Direction direction);
	virtual Room* getCurrentRoom();
	virtual void printCurrentRoom(bool full);

public:
	AbstractGame() {};
	virtual ~AbstractGame(void) {};
};
