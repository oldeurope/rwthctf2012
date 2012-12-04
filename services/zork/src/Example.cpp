#include "Example.h"
#include "ZorkEngine/model/Direction.h"

#ifndef WIN32
#include <cstring>
#define INT_MAX 0xFFFFFFFF
#endif

extern bool debugmodeon;


void Example::init()
{
	AbstractGame::init();
	AbstractGame::addCommand((AbstractCommand*)new SearchCommand());
	AbstractGame::addCommand((AbstractCommand*)new HideCommand());
	AbstractGame::addCommand((AbstractCommand*)new WriteCommand());
}

void Example::createRooms()
{
    //create rooms (names and descriptions)
	YourRoom = new Room("Your Room","You are in your room. You should tidy up sometimes. Stuff is everywhere. If your  enteres the Room and sees this you are so screwed. The exit is on the east side of your room.");
	Home = new Room("Entrance area","You are in the entrance area of your home. It's super tidy in here!! Of course. You can leave the house through the door (no way he just said that - but its on the northeast) or use the backdoor on the south. Of course you can reenter your room and CLEAN UP THE MESS, just kiddin' you're dont gonna do that on a weekend...");
	YourGarden = new Room("Your Garden","You are standing in your garden. Sweet Childhood dreams. All the fun you had in this place. Building sand castles. Playing hide and seek with the neighbour daughter. Building a tree house with your dad. Drinking secretly your first beer with your friends. And all that other childhood stuff. By the way the sexy neighbours daughter is now your girlfriend. Remember to go shopping with her this weekend!");

	Street = new Room("Street","You are now in the streets of this little town. You can go anywhere you want. Seriously.");

	GFHome = new Room("Your girlfriends House","You are entering your girlfriends House. Your girlfriends mom is standing in your way telling you to come back later. Ofcourse she talks to you like 45 min telling you all the latest gossip you never wanted to know but I stripped that out for you.");
	GFGarden = new Room("Neighbours Garden","A quick jump over the fence and you are standing in your neighbours garden. That \"quick jump\" took you five minutes of trying and you nearly ripped your trousers. You should really think about getting in shape again... Jesus ... Anyway you can see your girlfriends room from here to the north. You pick up some stones and try to throw them against the window. They go right through. No, you haven't chrushed the window, it was already open - idiot. That climbing has really drained you, right?");
	GFRoom = new Room("You girlfriends bedroom","You are in your girlfriends bedroom. That one is just beeing renovated. Therefore it is not that much to find here. Just a place to sleep and boxes full of clothes and stuff");

	ComputerStore = new Room("Computer Store","You entered your \"prefered\" computer store. Of course you hate it. Who does not buy computer stuff online anyway. No one seems to be here. You just here some noices out of the backroom. Here is much stuff. I guess you are here to buy a new battery for your Laptop but I dont think you can afford it. If you think the same as I do you should make sure that you are a better runner than climber");
		
	Pub = new Room("Local Pub","SMOKE is burning the eyes. MUSIC is floating your ears. DARKNESS nevermind its not that dark. You are in your local pub. Your friends are also here to make party with you. You drink a lot, you party a lot. But whats going on with that one of your friends who always drinks to much. Whats his name again? Anyway you know whom I talking about.");
	BackAlley = new Room("Back Alley","Fresh and cold air refresh your brain. You are now in the Back Alley of the local pub. You always end here if someone is about to throw up and doesn't feel all to good. Someone? Its always Jeff or whatever name he has");

	EnemyHouse = new Room("Your Worst Enemys House","You entered the house of that guy you ever hated since you can remember. And no one is here. Thats looks like the perfect chance for evil revenge.");

	Shoestore = new Room("Shoestore","You can buy shoes in here. I hope you did not enter together with your girlfriend. You know - there is no escape from a shoestore if you enter it with a girl you really like! OH my GOD there is a dead body of a man lying on the ground and there a dead body of a girl and a USP.45 tactical he must have killed himself and then his wife has killed herself too because she did not remeber the pin of her husbands credit card and she was not able to afford these shoes. Consider that as warning.");

	OldLibrary = new Room("Old mysterious Library","You climb through a broken window and you entered a magical place. A place where people used to go to find information. How stupid sounds that. Seems like no one set a food in here since forever.Books Everywhere! But lots of them are riped into pieces. Pages are lying all arround. Countless. This looks like a good place to hide information or to look for foreign secrets");

	Party = new Room("Street party","A Streetparty is going on here. The whole neighbourhood seems to be here. Also your archenemy. The kid you always hated for a good reason! Yes the reason is good.");

    //add exits for each room.  You need to add both sides of an individual exit, unless of course
    //it is a one way door (like a magical door)

	YourRoom->addExit(Direction::east,Home); Home->addExit(Direction::west,YourRoom);
	Home->addExit(Direction::south,YourGarden);YourGarden->addExit(Direction::north,Home);
	YourGarden->addExit(Direction::east,GFGarden);GFGarden->addExit(Direction::west,YourGarden);
	GFGarden->addExit(Direction::north,GFRoom);GFRoom->addExit(Direction::south,GFGarden);
	Home->addExit(Direction::northeast,Street);Street->addExit(Direction::southwest,Home);
	GFHome->addExit(Direction::north,Street);Street->addExit(Direction::south,GFHome);
	Shoestore->addExit(Direction::northwest,Street);Street->addExit(Direction::southeast,Shoestore);
	OldLibrary->addExit(Direction::west,Street);Street->addExit(Direction::east,OldLibrary);
	EnemyHouse->addExit(Direction::southwest,Street);Street->addExit(Direction::northeast,EnemyHouse);
	Pub->addExit(Direction::south,Street);Street->addExit(Direction::north,Pub);
	Pub->addExit(Direction::north,BackAlley);BackAlley->addExit(Direction::south,Pub);
	ComputerStore->addExit(Direction::southeast,Street);Street->addExit(Direction::northwest,ComputerStore);
	Party->addExit(Direction::east,Street);Street->addExit(Direction::west,Party);

    //start game where?
	currentRoom = YourRoom;
}
void Example::createItems()
{
    //create items and add to the initial room
	
	vector<string> tmp;


	tmp.clear();
	tmp.push_back("pc");
	tmp.push_back("computer");
	YourComputer = new Item(ItemType::Misc,false,false,
							"computer",tmp,
							"Your desktop PC is lying there in pieces. Your power supply is broken",
							"Your Windows 7 Box. Windows is awesome and everyone knows it.");
	YourRoom->addItem(YourComputer);
	

	tmp.clear();
	tmp.push_back("socks");
	tmp.push_back("dirty socks");	
	YourSocks = new Item(ItemType::Misc,true,true,
							"dirty socks",tmp,
							"Your Dirty socks are lying all over your floor, bed, desk...",
							"Its socks. They are dirty.");
	YourRoom->addItem(YourSocks);
	

	tmp.clear();
	tmp.push_back("shovel");
	tmp.push_back("garden shovel");	
	YourGardenShovel = new Item(ItemType::Misc,true,true,
							"shovel",tmp,
							"Your old shovel lies near the sandbox",
							"Build all the sand castles!!");
	YourGarden->addItem(YourGardenShovel);
	

	tmp.clear();
	tmp.push_back("sandbox");
	tmp.push_back("box");	
	YourGardenSandbox = new Item(ItemType::Misc,false,false,
							"sandbox",tmp,
							"Your sandbox stands in the back",
							"You always loved that one as a kid. Every kid does");
	YourGarden->addItem(YourGardenSandbox);
	

	tmp.clear();
	tmp.push_back("gf");
	tmp.push_back("girlfriend");	
	Girlfriend = new Item(ItemType::Misc,true,true,
							"girlfriend",tmp,
							"Your gorgeus looking girlfriend. Damn she is pretty. No one knows how you got this lucky.",
							"Your gorgeus looking girlfriend. Damn she is pretty. No one knows how you got this lucky.");
	GFRoom->addItem(Girlfriend);
	

	tmp.clear();
	tmp.push_back("mattress");
	tmp.push_back("bed");	
	Matrace = new Item(ItemType::Misc,false,false,
							"mattress",tmp,
							"A mattress is lying on the ground. You can \"sleep\" on that one.",
							"A mattress is lying on the ground. You can \"sleep\" on that one. Its a real special high tech one...");
	GFRoom->addItem(Matrace);
	

	tmp.clear();
	tmp.push_back("supply");
	tmp.push_back("power supply");	

	Battery = new Item(ItemType::Misc,true,true,
							"power supply",tmp,
							"There is a power supply on the main desk. Seems like exactly what you need.",
							"Excactly the power supply you need in order to repair your pc.");
	ComputerStore->addItem(Battery);
	

	tmp.clear();
	tmp.push_back("friend");
	tmp.push_back("drunk friend");	
	tmp.push_back("damn drunk friend");	

	Friend = new Item(ItemType::Misc,true,true,
							"damn drunk friend",tmp,
							"One of your friends is really drunk and falls to the floor",
							"every god damn time...");
	Pub->addItem(Friend);
	

	tmp.clear();
	tmp.push_back("bucket");
	Bucket = new Item(ItemType::Misc,true,true,
							"bucket",tmp,
							"A simple bucket.",
							"A simple bucket.");
	BackAlley->addItem(Bucket);
	

	tmp.clear();
	tmp.push_back("computer");
	tmp.push_back("pc");	

	EnemeyComputer = new Item(ItemType::Misc,false,false,
							"computer of that dick",tmp,
							"On that dest there is that fancy new computer that you wanted since forever.",
							"On that dest there is that fancy new computer that you wanted since forever.");
	EnemyHouse->addItem(EnemeyComputer);
	

	tmp.clear();
	tmp.push_back("dead male body");
	tmp.push_back("dead male");	
	tmp.push_back("dead man");
	tmp.push_back("man");
	DeadBodyM = new Item(ItemType::Misc,false,false,
							"dead male body",tmp,
							"A dead mans body lies leans against the wall",
							"He's dead. He looks horrible.");
	Shoestore->addItem(DeadBodyM);
	

	tmp.clear();
	tmp.push_back("dead female body");
	tmp.push_back("dead female");	
	tmp.push_back("dead woman");
	tmp.push_back("woman");
	DeadBodyF = new Item(ItemType::Misc,false,false,
							"dead female body",tmp,
							"A dead female body is near the counter",
							"She's dead. She still looks hot.");
	Shoestore->addItem(DeadBodyF);
	

	tmp.clear();
	tmp.push_back("pistol");
	tmp.push_back("gun");	
	tmp.push_back("usp");	
	tmp.push_back("USP.45 Tactical");
	Gun = new Item(ItemType::Misc,true,false,
							"USP.45 Tactical",tmp,
							"An USP.45 Tactical is lying right in the middle of the room",
							"DANGER! Smoking kills people. Ehm guns too.");
	Shoestore->addItem(Gun);
	

	tmp.clear();
	tmp.push_back("shoe");
	tmp.push_back("shoes");	
	Shoes = new Item(ItemType::Misc,true,true,
							"Shoes",tmp,
							"Fancy shoes are everywhere",
							"Woman shoes. I dont know how to better describe shoes.");
	Shoestore->addItem(Shoes);
	


	tmp.clear();
	tmp.push_back("big old book");
	tmp.push_back("book");
	OldBook = new Item(ItemType::Misc, true, true, "big old book",
			tmp,
            "Many books are everywhere",
            "You could leave a secret message on one of the pages of a book");
	OldLibrary->addItem(OldBook);

}


bool Example::removeInventoryItem(Item* item, bool permanent)
{	
	if (item == Girlfriend && (currentRoom != Shoestore || bTookGFShopping))
	{
		// add exit
		Shoestore->addExit(Direction::northwest,Street);
	}
	return 	AbstractGame::removeInventoryItem(item,permanent);
}


extern char name[];

void Example::addToInventory(Item* item)
{
	AbstractGame::addToInventory(item);

    
	if (item == Girlfriend)
	{
		Shoestore->addExit(Direction::northwest,0);
	}

	if (item == Gun)
	{
		addCommand(new ExecuteCommand());
	}

}

extern bool hadRevenge;

void Example::combineItems(Item* appliedTo, Item* toApply)
{
    //here we evalute "use thing1 on thing2", which in this case, you can see
    //that it results in a new item being created

	int counter = 0;
	bHadRevenge = hadRevenge;

/////////// fun stuff start

	if ((  appliedTo == YourGardenSandbox && toApply == YourGardenShovel
		|| appliedTo == YourGardenShovel && toApply == YourGardenSandbox)
		)
	{
		cout << "You build a wonderfull sandcastle! Damn all that childhood practice really payed off on the end! You should do that professionally." << endl;	
	}

	else if ((  appliedTo == Bucket && toApply == Girlfriend
		|| appliedTo == Girlfriend && toApply == Bucket)
		)
	{
		cout << "You put the bucket on your head and start moving like a zombie. Your girlfriend laughs." << endl;	
	}

	else if ((  appliedTo == Battery && toApply == Girlfriend
		|| appliedTo == Girlfriend && toApply == Battery)
		)
	{
		cout << "You sneak up to your girlfriend and pretend shocking her with the supply. She gets really mad!" << endl;	
	}

	else if ((  appliedTo == YourGardenShovel && toApply == Girlfriend
		|| appliedTo == Girlfriend && toApply == YourGardenShovel)
		)
	{
		cout << "You hit your girlfriend with the little shovel. She gets really mad and still wet ;o" << endl;	
	}


	else if ((  appliedTo == YourSocks && toApply == Girlfriend
		|| appliedTo == Girlfriend && toApply == YourSocks)
		)
	{
		cout << "You try to sneak up to your girlfriend and scare her with your dirty socks. She disarms you and puts them into your mouth." << endl;	
	}


	else if ((  appliedTo == YourGardenShovel && toApply == Bucket
		|| appliedTo == Bucket && toApply == YourGardenShovel)
		)
	{
		cout << "You take the shovel and put them into the bucket. Now you look like a 5 year old. You immediatly remove the shovel from the bucket" << endl;	
	}


/////////// fun stuff end

	// repair computer
	else if ((  appliedTo == Battery && toApply == YourComputer
		|| appliedTo == YourComputer && toApply == Battery)
		&& !bRepairedComputer)
	{
		cout << "Finally the computer is working again."<< endl;
		cout << "The computer is starting up: Its resuming your last session" << endl
			 << "There we go... "<< endl
			 << "Seems like the last page you visited tells you how to format a drive" << endl
			 << "Ahhhh!! You now understand the command!"<<endl;
		addCommand(new FormatCommand());
		bRepairedComputer = true;
	
	}


	// go to sleep with gf
	else if ((  appliedTo == Matrace && toApply == Girlfriend
		|| appliedTo == Girlfriend && toApply == Matrace)
		)
	{
		cout << "Im not going to commentate on what you did right now. But. Give me a High Five :D " << endl;
		if (!bSleeped)
			bSleeped = true;
		else
			cout << "and again and again ;) "<<endl;
	}


	// drink with buddies
	else if ((  appliedTo == Bucket && toApply == Friend
		|| appliedTo == Friend && toApply == Bucket)
		&& !bDrankWithBuddies)
	{
		cout << "aowiealskdhajwhekja! ajshdkashdjhwo! He throwes up into the bucket. That's again the end of a legendary night with the guys!" << endl;
		bDrankWithBuddies = true;
	}


	// buy gf shoes
	else if ((  appliedTo == Shoes && toApply == Girlfriend
		|| appliedTo == Girlfriend && toApply == Shoes)
		)
	{
		cout << "Oh my god she's so happy she can't get enough shoes!" << endl;
		bTookGFShopping = true;
	}

	else
	{
		debugmodeon = -debugmodeon+1; // BUG ONE
		cout << "Those items can't be used in that fashion"<<endl;
		return;
	}
	

	if (bHadRevenge)
		++counter;

	if (bSleeped)
		++counter;

	if (bDrankWithBuddies)
		++counter;

	if (bRepairedComputer)
		++counter; 

	if (bTookGFShopping)
		++counter;

    if (counter == 5) {
        //Game over
		cout << "Congratulations! You did all the Teenager stuff also this weekend You are now pronounced " ;
		char title[] = "Sir. ";
		memmove(name+strlen(title),name,strlen(name)+1);
		memcpy(name,title,strlen(title));
		cout << name << endl;
		clearScreen();
		cout << "The end."<< endl;
    }
}

void Example::printWelcome()
{
    //print the initial text to get the story started
    cout << "Have fun. One tip in advance: do human fuzzing. (Just play the game and make educated guesses)" << endl;
    cout << "Type 'help' if you need help. Type 'help more' if you need more help"<<endl;
    cout << endl;
	cout << "You wakeup. Its weekend! You are in your room. You drank so much that night that not even I can remember your name" << endl;
	cout << "Whats you name?\n> ";

	cin.getline(name,30);
	if (strnlen(name,30) >= 29)
	{
		cin.clear();
		cin.ignore(INT_MAX,'\n');
	}
	
	cout << "have fun again " << name<< endl;

    printCurrentRoom(true);
}

Example::Example(void)
{
	instance = this;


	bHadRevenge = false;
	bRepairedComputer = false;
	bDrankWithBuddies = false;
	bSleeped = false;
	bTookGFShopping = false;
}

Example::~Example(void)
{
}
