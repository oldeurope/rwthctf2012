#include "Item.h"

string Item::getName() {
    return itemName;
}

string Item::getOriginalDescription() {
    return origDescription;
}

vector<string> Item::getKeywords() {
    return keywords;
}

ItemType Item::getType() {
    return type;
}

string Item::examine() {
    return itemDetails;
}

bool Item::canInventory() {
    return m_canInventory;
}

bool Item::canDrop() {
    return m_canDrop;
}

void Item::setDropped(bool dropped) {
    m_wasDropped = dropped;
}

bool Item::wasDropped() {
    return m_wasDropped;
}

bool Item::keywordMatch(string itemName) {
	for (unsigned int i = 0; i < keywords.size(); ++i)
		if (keywords[i] == itemName)
			return true;

	return false;
}


Item::Item(ItemType type, bool canInventory, bool canDrop,string itemName, vector<string> keywords, string origDescription,string itemDetails)
{
    this->keywords = keywords;
    this->itemName = itemName;
    this->type = type;
    this->origDescription = origDescription;
    this->itemDetails = itemDetails;
    this->m_canInventory = canInventory;
    this->m_canDrop = canDrop;
}

Item::~Item(void)
{
}
