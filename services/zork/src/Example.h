#pragma once

#include "ZorkEngine/model/Direction.h"
#include "ZorkEngine/model/Room.h"
#include "ZorkEngine/model/Item.h"
#include "ZorkEngine/model/ItemType.h"

#include "HideCommand.h"
#include "SearchCommand.h"
#include "WriteCommand.h"
#include "FormatCommand.h"
#include "ExecuteCommand.h"

#include "ZorkEngine/AbstractGame.h"


class Example : public AbstractGame
{
public:

    //instance variables of every room in the game
    Room *YourRoom,
		 *Home,
		 *YourGarden,

		 *Street,

		 *GFHome,
		 *GFGarden,
		 *GFRoom,

		 *ComputerStore,
		 
		 *Pub,
		 *BackAlley,

		 *EnemyHouse,

		 *Shoestore,

		 *OldLibrary,

		 *Party
         ;

    //instance variables of every item in the game
    Item *YourComputer,
		 *YourSocks,

		 *YourGardenShovel,
		 *YourGardenSandbox,

		 *Girlfriend,
		 *Matrace,

		 *Battery,

		 *Friend,
		 *Bucket,

		 *EnemeyComputer,

		 *DeadBodyM,
		 *DeadBodyF,
		 *Gun,
		 *Shoes,

		 *OldBook
         ;

	bool bHadRevenge;
	bool bRepairedComputer;
	bool bDrankWithBuddies;
	bool bSleeped;
	bool bTookGFShopping;


	virtual void init();
	virtual void createRooms();
	virtual void createItems();
	virtual void addToInventory(Item* item);
	virtual bool removeInventoryItem(Item* item, bool permanent);
	virtual void combineItems(Item* appliedTo, Item* toApply);
	virtual void printWelcome() ;

	Example(void);
	~Example(void);
};
