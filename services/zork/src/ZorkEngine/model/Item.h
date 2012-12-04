#pragma once



#include <string>
#include <vector>
#include <algorithm>

using namespace std;

#include "ItemType.h"

class Item
{
private:
	vector<string> keywords;
	string itemName;
	string origDescription;
	string itemDetails;
	ItemType type;
	bool m_canInventory;
	bool m_canDrop;
	bool m_wasDropped;
	

public:

	void setNewItemDetails(string s) {itemDetails =  s; }
	void setNewItemName(string s) {itemName = s;}

    string getName();

    string getOriginalDescription();

    vector<string> getKeywords();

    ItemType getType();

    string examine();

    bool canInventory();

    bool canDrop();

	void setDropped(bool dropped);

    bool wasDropped();

    bool keywordMatch(string itemName);

	Item(ItemType type, bool canInventory, bool canDrop,string itemName, vector<string> keywords, string origDescription,string itemDetails);
	virtual ~Item(void);
};
