#pragma once

#include "Direction.h"

#include <string>
#include <vector>
#include <map>

#include "Item.h"

using namespace std;

class Room;

class Room
{
private:
    string description;
    string extendedDescription;
    map<Direction,Room*> exits;
    vector<Item*> items;
    bool wasVisited;


	string exitstring();

public:
    void setExits(Room* north, Room* east, Room* south, Room* west);
    
    void addExit(Direction direction, Room* room);

    void addItem(Item* item);

    void removeItem(Item* item);

    Item* getItem(string itemName);

    string shortDescription();

    string longDescription(bool fullDescription);

    Room* nextRoom(Direction direction);

	Room(string description, string extendedDescription);
	~Room(void);
};
