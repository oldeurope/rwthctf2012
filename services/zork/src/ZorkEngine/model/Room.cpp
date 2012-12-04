#include "Room.h"


void Room::addExit(Direction direction, Room* room) {
    exits[direction] =  room;
}

void Room::addItem(Item* item) {
    items.push_back(item);
}

void Room::removeItem(Item* item) {
	if (find(items.begin(),items.end(),item) != items.end())
		items.erase(find(items.begin(),items.end(),item));
}

Item* Room::getItem(string itemName) {
	for (unsigned int i=0; i<items.size(); i++)
		if (items[i]->keywordMatch(itemName))
			return items[i];
	return NULL;
}

string Room::shortDescription() {
    return description;
}

string Room::longDescription(bool fullDescription) {
    if (extendedDescription != "" && (!wasVisited || fullDescription)) {
        wasVisited = true;
        string itemdescripts = "";
		for (unsigned int i = 0; i < items.size();++i)
		{
            itemdescripts += "\n" + ((items[i]->wasDropped()) ?
                                     "A " + items[i]->getName() + " is lying on the ground" :
                                     items[i]->getOriginalDescription());			
		}

        return extendedDescription + itemdescripts;
    } else {
        string itemlist = "";
		for (unsigned int i = 0; i < items.size();++i)
		{
            itemlist += "\n" + items[i]->getName();
        }
        return ((itemlist.length() > 0) ? "\nThe room contains:" + itemlist : "");
    }
}


Room* Room::nextRoom(Direction direction) {
    return exits[direction];
}



void Room::setExits(Room* north, Room* east, Room* south, Room* west) {
        if(north != NULL)
			exits[Direction::north]= north;
        if(east != NULL)
			exits[Direction::east]= east;
        if(south != NULL)
			exits[Direction::south]= south;
        if(west != NULL)
			exits[Direction::west]= west;
    }
    

string Room::exitstring() 
{
	
    string returnstring = "Exits:";
	for(map<Direction,Room*>::iterator i = exits.begin(); i != exits.end(); ++i)
	{
		returnstring += " " + (*i).first;
	}

    return returnstring;
	
}

Room::Room(string description, string extendedDescription)
{
    this->description = description;
    this->extendedDescription = extendedDescription;
	wasVisited = false;
}

Room::~Room(void)
{
}
