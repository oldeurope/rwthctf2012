#include "AbstractGame.h"

AbstractGame* AbstractGame::instance = 0;

void AbstractGame::init()
{

  createRooms();
  createItems();
  addBaseCommands();
  parser = new Parser(this);
 
}

void AbstractGame::destroy()
{
  delete instance;
  instance = 0;
}


vector<AbstractCommand*> AbstractGame::getCommandExecutors(){
	return executors;
}

void AbstractGame::addBaseCommands() {
    executors.push_back((AbstractCommand*)new NavigateCommand());
    executors.push_back((AbstractCommand*)new DropCommand());
    executors.push_back((AbstractCommand*)new ExamineCommand());
    executors.push_back((AbstractCommand*)new InventoryCommand());
    executors.push_back((AbstractCommand*)new TakeCommand());
    executors.push_back((AbstractCommand*)new ReadCommand());
    executors.push_back((AbstractCommand*)new UseCommand());
    executors.push_back((AbstractCommand*)new HelpCommand());
}

void AbstractGame::addCommand(AbstractCommand* command) 
{
    executors.push_back(command);
}

void AbstractGame::addToInventory(Item* item) 
{
	for (unsigned int i = 0; i < inventory.size(); ++i)
	{
		if (inventory[i] == item)
		{
			//cout << "This exact same Item is in your Inventory"<<endl;
			return;
		}
	}

	item->setDropped(false);
	inventory.push_back(item);
	currentRoom->removeItem(item);
	cout <<  "The " << item->getName() << " was added to your inventory"<< endl; 
}


Item* AbstractGame::getInventoryItem(string itemName) 
{ 	
	for (unsigned int i = 0; i < inventory.size(); ++i)
	{
		if (inventory[i]->keywordMatch(itemName))
			return inventory[i];
	}

	return NULL;
}

bool AbstractGame::removeInventoryItem(Item* item, bool permanent) 
{
    if (item->canDrop()) {
        if (!permanent) {
            item->setDropped(true);
            currentRoom->addItem(item);
        }
        inventory.erase(find(inventory.begin(),inventory.end(),item));
        return true;
    } else {
        return false;
    }
	return false;
}

void AbstractGame::play() {            
    printWelcome();
	parser->run();
}

void AbstractGame::printWelcome() {
    //override if necc
    cout << "Type 'help' if you need help."<< endl;
    printCurrentRoom(true);
}


void AbstractGame::printHelp(bool SupressSyns) {
    cout << "The available commands are:" << endl;
    string allcommands = "";

	for (unsigned int i= 0 ; i< executors.size() ;++i)
	{
		vector<string> tmp = executors[i]->getRecognizedCommands();
		for (unsigned int j = 0; j <  (SupressSyns ? 1 : tmp.size()) ; ++j)
		{
			 allcommands += " - " + tmp[j];
		}
		allcommands += "\n\n";
	}

    cout << allcommands << endl;
	
}

void AbstractGame::clearScreen() {
	cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
}

bool AbstractGame::processCommand(Command command) 
{
	for (unsigned int i= 0 ; i<  executors.size();++i)
	{
		vector<string> tmp = executors[i]->getRecognizedCommands();
		for (unsigned int j = 0; j < tmp.size(); ++j)
		{
			if (tmp[j] == command.getCommandWord())
				return executors[i]->execute(command);
		}
	}
	return false;
}

void AbstractGame::printInverntory() {

    string allitems = "";
	for (unsigned int i = 0; i < inventory.size();++i)
	{
		allitems += "\n" + inventory[i]->getName();
	
	}

    if (allitems.length() > 0) {
        cout << "Your inventory: " << allitems<< endl;
    } else {
        cout << "Your inventory is empty"<< endl;
    }
	
}

void AbstractGame::goInDirection(Direction direction) {
  
    // Try to leave current room.
    Room* nextRoom = currentRoom->nextRoom(direction);

    if (nextRoom == NULL) {
        cout << "You can't go that way!" << endl;
    } else {
        currentRoom = nextRoom;
        printCurrentRoom(false);
    }
	
}

Room* AbstractGame::getCurrentRoom() {
    return currentRoom;
}

void AbstractGame::printCurrentRoom(bool full) {
    string longDesc = currentRoom->longDescription(full);
    if (longDesc != "") {
        cout << currentRoom->shortDescription() << "\n" << longDesc << endl;
    } else {
        cout << currentRoom->shortDescription()<< endl;
    }
}
