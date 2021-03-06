//Matthew Dyer and Michael Lee
//gameloop function

#include <iostream>
#include <vector>
#include <queue>
#include "gameLoop.h"
#include "Library/Planet.h"
#include "Library/player.h"
#include "Library/army.h"
#include "battle.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <thread>

using namespace std;

//I also hate this, but I have no real choice
bool canFlip;
vector<Planet*>* altPlan;
vector<string>* altMap;
Player* altCur;
void reinforce(Player* player, int bonus);
void gamePrep(queue<Player*> players, vector<Planet*> planets, vector<string> map);
void* flippies(void* arg);

void gameLoop(queue<Player*> players, vector<Planet*> planets, vector<string> map){

	seed();
	altPlan = &planets;
	altMap = &map;
	gamePrep(players,planets,map);
	canFlip=false;
	//allegro graphics beginning here
	//information and help found at allegro.cc
	if(!al_init()){
		cout << "Failed to initialize display.\n";
		exit(-1);
	}
	if(!al_init_font_addon()){
		cout << "Failed to initialize font addon.\n";
		exit(-1);
	}
	if(!al_init_ttf_addon()){
		cout << "Failed to initialize ttf addon.\n";
		exit(-1);
	}
	if(!al_init_image_addon()){
		cout << "Failed to initialize image addon.\n";
		exit(-1);
	}
	if(!al_init_primitives_addon()){
		cout << "Failed to initialize primitives addon.\n";
		exit(-1);
	}

	//color mapping
	ALLEGRO_COLOR red = al_map_rgb(202,72,10);
	ALLEGRO_COLOR green = al_map_rgb(17,105,17);
	ALLEGRO_COLOR white = al_map_rgb(255,255,255);
	ALLEGRO_COLOR brown = al_map_rgb(85,40,30);
	ALLEGRO_COLOR otherBrown = al_map_rgb(110,60,50);

	//load font
	ALLEGRO_FONT *font = al_load_ttf_font("Assets/Xolonium-Regular.ttf", 16,0);
	if(!font){
		cout << "Error grabbing font\n";
		exit(-1);
	}

	int w = 0;
	int h = 0;
	const int scale = 20;

	//determine map size for allegro
	if(players.size() == 2){
		w = 25*scale;
		h = 25*scale;
	}else if(players.size() == 3){
		w = 30*scale;
		h = 25*scale;
	}else if(players.size() == 4){
		w = 30*scale;
		h = 30*scale;
	}else if(players.size() == 5){
		w = 35*scale;
		h = 30*scale;
	}else if(players.size() == 6){
		w = 35*scale;
		h = 35*scale;
	}

	//create display
	ALLEGRO_DISPLAY *display = al_create_display(w,h);
	if(!display){
		cout << "Error creating display\n";
		exit(-1);
	}
	pthread_t tid;
	pthread_create(&tid, NULL, &flippies, (void*) display);

	//set up a random bg image
	ALLEGRO_BITMAP *image = NULL;

	int pickBG = rand()%5;
	if(pickBG == 0){ 
		image = al_load_bitmap("Assets/bg1.bmp");	
	}else if(pickBG == 1){	
		image = al_load_bitmap("Assets/bg2.bmp");	
	}else if(pickBG == 2){	
		image = al_load_bitmap("Assets/bg3.bmp");
	}else if(pickBG == 3){	
		image = al_load_bitmap("Assets/bg4.bmp");
	}else if(pickBG == 4){	
		image = al_load_bitmap("Assets/bg5.bmp");
	}

	al_draw_bitmap(image,0,0,0);
//tag52
	for(unsigned int i=0; i<planets.size(); i++)
	{
	  int x1, y1, x2, y2;
	  for(int j=0; j<map.size(); j++)
	    for(int k=0; k<map[j].size(); k++)
	      if(map[j][k] == planets[i]->name()[0])
	      {
		y1 = j;
		x1 = k;
	      }
	  for(unsigned int j=0; j<planets[i]->allConnections().size(); j++)
	  {
	    for(int k=0; k<map.size(); k++)
	    {
	      for(int l=0; l<map[j].size(); l++)
	      {
		if(map[k][l] == planets[i]->allConnections()[j][0])
		{
		  y2 = k;
		  x2 = l;
		  al_draw_line(x1*scale+10,y1*scale+10,x2*scale+10,y2*scale+10,white,1);
		}
	      }
	    }
	  }
	}
	for(int i = 0; i < map.size(); ++i){
		for(int j = 0; j < map[i].size(); ++j){
			if(map[i][j] >= 65 && map[i][j] <= 90){
				string name = "";
				name.push_back(map[i][j]);
				al_draw_filled_circle(j*scale+10,i*scale+10,15,red);
				al_draw_text(font,white,j*scale+5,i*scale+5,0,name.c_str());
			}
			if(map[i][j] == '*'){
				int pickcol = rand()%2;
				if(pickcol){
					al_draw_filled_circle(j*scale+10,i*scale+10,4,brown);
				}else{
					al_draw_filled_circle(j*scale+10,i*scale+10,4,otherBrown);
				}
			}
		}
	}
	al_flip_display();
	canFlip=true;

	int turn = 0;
	string command = "";
	string planet1 = "";
	string planet2 = "";
	int size = 0;
	int bonus=0;
	string banner = "----------------------------------------------------------\n|********** Taking Chances on a Galactic Scale **********|\n----------------------------------------------------------\n";

	while(players.size() > 1){ 

		//here's where the fun begins
		Player* current = players.front();
		altCur=current;
		if(current->isDead())
		{
			players.pop();
			continue;
		}

		cout << "\033[2J\033[1;1H";
		cout << banner << endl;
		cout << "Your turn " << current->name << "\n";

		//REPLACE THIS ONCE GRAPHICS HAS LABELS AND STUFF
		for(unsigned int i = 0; i < map.size(); ++i){
			cout << map[i] << "\n";
		}

		cout << "You own the following planets: \n";
		vector<Planet*> playerPlans = current->planetsHeld();
		vector<string> planetNames; //push all owned planets into here for coloration
		for(unsigned int i=0; i<playerPlans.size(); i++){
			cout << playerPlans[i]->name() << " ";
			planetNames.push_back(playerPlans[i]->name());
			cout << playerPlans[i]->armiesHeld() << "\n";
		}

		//redraw map for coloration again
		if(canFlip){
		canFlip=false;
		for(int i = 0; i < map.size(); ++i){
			for(int j = 0; j < map[i].size(); ++j){
				if(map[i][j] >= 65 && map[i][j] <= 90){
					string name = "";
					name.push_back(map[i][j]);
					al_draw_filled_circle(j*scale+10,i*scale+10,15,red);
					al_draw_text(font,white,j*scale+5,i*scale+5,0,name.c_str());
				}
				/*if(map[i][j] == '*'){
					int pickcol = rand()%2;
					if(pickcol){
						al_draw_filled_circle(j*scale+10,i*scale+10,4,brown);
					}else{
						al_draw_filled_circle(j*scale+10,i*scale+10,4,otherBrown);
					}
				}*/
			}
		}
		//al_flip_display();

		//planet coloration for player --player planets are green
		for(int i = 0; i < map.size(); ++i){
			for(int j = 0; j < map[i].size(); ++j){
				if(map[i][j] >= 65 && map[i][j] <= 90){
					string name = "";
					name.push_back(map[i][j]);
					al_draw_filled_circle(j*scale+10,i*scale+10,15,red);
					al_draw_text(font,white,j*scale+5,i*scale+5,0,name.c_str());
					for(int k = 0; k < planetNames.size(); ++k){
						if(planetNames[k] == name){
							al_draw_filled_circle(j*scale+10,i*scale+10,15,green);
							al_draw_text(font,white,j*scale+5,i*scale+5,0,name.c_str());
						}
					}
				}
				/*if(map[i][j] == '*'){
					int pickcol = rand()%2;
					if(pickcol){
						al_draw_filled_circle(j*scale+10,i*scale+10,4,brown);
					}else{
						al_draw_filled_circle(j*scale+10,i*scale+10,4,otherBrown);
					}
				}*/
			}
		}

		al_flip_display();
		//al_rest(1);
		canFlip=true;
		}


		cout << "Turn " << turn << "\n";

		//We really need to cout Map and armies at some point
		bool canAttack=true;
		bool canMove=true; //only one attack and move
		while(canAttack or canMove){
			cout << "Enter a command:\n";

			while(cin >> command){

				if(command != "help" && command != "quit" && command != "end" && command != "view" && command != "attack" && command != "move"){
					cout << "Not a valid command. Type 'help' for a list of commands.\n";
				}else{
					break;
				}
			}

			if(command == "help"){
				//print out help menu
				cout << "The following are valid commands:\n";
				cout << "\t- attack [from this planet] [to this planet] [size of army]\n";
				cout << "\t\t-> example: attack A C 50\n";
				cout << "\t- move [from this planet] [to this planet] [size of army]\n";
				cout << "\t\t-> example: move A B 25\n";
				cout << "\t- view\n";
				cout << "\t\t-> view game information\n";
				cout << "\t- end\n";
				cout << "\t\t-> used to end your turn\n";
				cout << "\t- quit\n";
				cout << "\t\t-> quit the game\n";
			}else if(command == "quit"){
				cout << "Thanks for playing!\n";
				al_destroy_display(display);
				al_destroy_bitmap(image);
				exit(0); //we're going to want to just end the whole program at this point.
			}else if(command == "end"){	
				//change the player and increment turn
				canAttack=false;
				canMove=false;
				//it's easier to auto-end turns like this

			}else if(command == "view"){
				cout << "Planet:\tUnits:\tOwner:\n";
				for(unsigned int i=0; i<planets.size(); i++){
					cout << planets[i]->name() << "\t";
					cout << planets[i]->armiesHeld() << "\t";
					cout << planets[i]->whoOwnsPlanet() << "\n";
				}

			}else{ //read in the rest
				cin >> planet1;
				cin >> planet2;
				cin >> size;
				if (cin.fail())
				{
					cout<<"Please enter a number for number of units.\n";
					cin.clear();
					cin.get(); //cleaning up cin is a pain...
					continue;
				}
				vector<Planet*> playerPlans = current->planetsHeld();
				Planet* plan1;
				bool isOwned=false;
				for(unsigned int i=0; i<playerPlans.size(); i++)
					if(playerPlans[i]->name()==planet1)
					{
						isOwned=true;
						plan1= playerPlans[i];
						break;
					}
				if(!isOwned)
				{
					cout << "Please input a planet you own for the planet being moved from\n";
					continue;
				}
				if(size >= plan1->armiesHeld())
				{
					cout << "You must leave at least one unit behind\n";
					continue;
				}
				if(command == "attack"){
					if(!canAttack)
					{
						cout << "You have already attacked this turn.  Please move or end your turn.\n";
						continue;
					}
					bool isValid=true;
					for(unsigned int i=0; i<playerPlans.size(); i++)
						if(playerPlans[i]->name()==planet2)
						{
							isValid=false;
							break;
						}
					if(!isValid)
					{
						cout <<"You can only attack planets you don't own\n";
						continue;
					}
					Planet* plan2;
					isValid=false;
					for(unsigned int i=0; i<planets.size(); i++)
						if(planets[i]->name()==planet2)
						{
							isValid=true;
							plan2= planets[i];
							break;
						}
					if(!isValid)
					{
						cout<<"Please input an opponent's planet to attack\n";
						continue;
					}
					isValid=false;
					vector<string> cons = plan1->allConnections();
					for(unsigned int i=0; i<cons.size(); i++)
					{
						if(cons[i]==plan2->name())
							isValid=true;
					}
					if(!isValid)
					{
						cout << "Those planets are not connected\n";
						continue;
					}
					cout << "Now attacking " << planet2 << " from " << planet1 << " with " << size << " units.\n";
					//call battle with our information
					//Now conveniently stored in plan1 and plan2!!!
					Army* battleArmy = new Army(size);
					plan1->armyHeld()->eliminate(size);

					bool result = battle(battleArmy, plan2->armyHeld());

					if(result){
						cout << "Victory! The planet is yours\n"<<battleArmy->size()<<" units survived!\n";
						current->numV++;
						if (current->numV == 3)
							bonus++;
						plan2->army = battleArmy;
						Player* loser;
						for(unsigned int i=0; i<players.size(); i++)
						{
							Player* temp = players.front();
							players.pop();
							players.push(temp);
							vector<Planet*> tempP = temp->planetsHeld();
							for(unsigned int j=0; j<tempP.size(); j++)
							{
								if (tempP[j]==plan2)
								{
									loser=temp;
								}
							}
						}
						loser->losePlanet(plan2);
						current->gainPlanet(plan2);
						plan2->setOwner(current->name);
						planetNames.push_back(plan2->name());
						//update map
						if(canFlip){
						canFlip=false;
						for(int i = 0; i < map.size(); ++i){
							for(int j = 0; j < map[i].size(); ++j){
								if(map[i][j] >= 65 && map[i][j] <= 90){
									string name = "";
									name.push_back(map[i][j]);
									al_draw_filled_circle(j*scale+10,i*scale+10,15,red);
									al_draw_text(font,white,j*scale+5,i*scale+5,0,name.c_str());
									for(int k = 0; k < planetNames.size(); ++k){
										if(planetNames[k] == name){
											al_draw_filled_circle(j*scale+10,i*scale+10,15,green);
											al_draw_text(font,white,j*scale+5,i*scale+5,0,name.c_str());
										}
									}
								}
								if(map[i][j] == '*'){
									int pickcol = rand()%2;
									if(pickcol){
										al_draw_filled_circle(j*scale+10,i*scale+10,4,brown);
									}else{
										al_draw_filled_circle(j*scale+10,i*scale+10,4,otherBrown);
									}
								}
							}
						}
						al_flip_display();
						canFlip=true;
						}
						if((loser->isDead()) && (players.size() == 2))
						{
							cout << "Congratulations, you won!\n";
							return;
						}
					}else{
						current->numV=0;
						cout << "You suffered a tragic defeat\n";
					}

					cout << "\nYou currently own the following planets: \n";
					vector<Planet*> playerPlans = current->planetsHeld();
					for(unsigned int i=0; i<playerPlans.size(); i++){
						cout << playerPlans[i]->name() << " ";
						cout << playerPlans[i]->armiesHeld() << "\n";
					}

					canAttack=false;//Note that player has made their attack
				}else if(command == "move"){
					if(!canMove)
					{
						cout << "You have already moved this turn.  Please attack or end your turn.\n";
						continue;
					}
					Planet* plan2;
					isOwned=false;
					for(unsigned int i=0; i<playerPlans.size(); i++)
						if(playerPlans[i]->name()==planet2)
						{
							isOwned=true;
							plan2= playerPlans[i];
							break;
						}
					if(!isOwned)
					{
						cout << "Please input a planet you own for the planet being moved to\n";
						continue;
					}
					bool isCon=false;
					vector<string> cons = plan1->allConnections();
					for(unsigned int i=0; i<cons.size(); i++)
					{
						if(cons[i]==plan2->name())
							isCon=true;
					}
					if(!isCon)
					{
						cout << "Those planets are not connected\n";
						continue;
					}
					cout << "Now moving " << size << " units from " << planet1 << " to " << planet2 << "\n";
					Army* temp = plan1->armyHeld()->split(size);
					plan2->armyHeld()->join(temp);

					cout << "\nYour planets after the move: \n";
					vector<Planet*> playerPlans = current->planetsHeld();
					for(unsigned int i=0; i<playerPlans.size(); i++){
						cout << playerPlans[i]->name() << " ";
						cout << playerPlans[i]->armiesHeld() << "\n";
					}
					canMove=false;
				}	
			}
			command = "";
		}
		reinforce(current,bonus);	
		players.pop();
		players.push(current);
		++turn;
		cout << "Your turn has ended\n";

	}

	al_destroy_display(display);

}

void reinforce(Player* player, int bonus)
{
	vector<Planet*> plans = player->planetsHeld();
	int rein = plans.size();
	bool bonusGet=false;
	if(player->numV == 3)
	{
		player->numV=0;
		bonusGet=true;
		rein+=(5*bonus);
	}
	if(rein>0)
		cout << "\nReinforcement phase:\nEnter '<planet> <number>' to reinforce\n";
	if(bonusGet)
		cout<<"Congratulations!  3 successive victory bonus of "<<5*bonus<<" units recieved!\n";
	while (rein >= 0)
	{
		if(rein == 0){
			cout << "You have "<< rein <<" reinforcements remaining\n";
			break;
		}
		cout << "You have "<< rein <<" reinforcements remaining\n";
		string planet;
		int num;
		cin >> planet;
		cin >> num;
		if (cin.fail())
		{
			cout<<"Please enter a number for number of units.\n";
			cin.clear();
			cin.get(); //cleaning up cin is a pain...
			continue;
		}
		Planet* plan;
		bool isOwned=false;
		for(unsigned int i=0; i<plans.size(); i++)
			if(plans[i]->name()==planet)
			{
				isOwned=true;
				plan = plans[i];
				break;
			}
		if(!isOwned)
		{
			cout << "Please input a planet you own\n";
			continue;
		}
		if(num>rein)
		{
			cout << "Please input a number no larger than "<< rein << endl;
			continue;
		}
		Army* army = plan->armyHeld();
		army->reinforce(num);
		rein = rein - num;
	}
	cout << "All reinforcements placed\n";
}

void gamePrep(queue<Player*> players, vector<Planet*> planets, vector<string> map)
{
	vector<int> assign;
	vector<int> orig;
	for(unsigned int i=0; i<planets.size(); i++)
	{
		orig.push_back(rand());
	}
	assign = orig;
	sort(assign.begin(), assign.end());
	int numP = players.size();
	Player* current;

	for (unsigned int i=0; i<orig.size(); i++)
	{
		current = players.front();
		players.pop();
		players.push(current);
		for (unsigned int j=0; j<assign.size(); j++)
		{
			if(orig[i] == assign[j])
			{
				if(planets[j]->whoOwnsPlanet() == "")
				{
					planets[j]->setOwner(current->name);
					current->gainPlanet(planets[j]);
					planets[j]->army = new Army(1);
				}
			}
		}
	}
	for(unsigned int i=0; i < players.size(); i++){
		current = players.front();
		players.pop();
		players.push(current);
		vector<Planet*> plans = current->planetsHeld();
		int rein = plans.size()*3;

		//Print out map. SWITCH WITH GRAPHICAL VERSION EVENTUALLY!
		cout << "\033[2J\033[1;1H";
		for(unsigned int i = 0; i < map.size(); ++i){
			cout << map[i] << "\n";
		}
		//Print out stuff the player owns	
		cout << "\nYou own the following:\n";
		cout << "Planet:\tUnits:\n";
		for(unsigned int i=0; i<plans.size(); i++){
			cout << plans[i]->name() << "\t";
			cout << plans[i]->armiesHeld() << "\n"; //This will always be one...
		}

		if(rein>0)
			cout << current->name << "'s initial army placement:\nEnter '<planet> <number>' to place armies\n";
		while (rein >= 0)
		{
			if(rein == 0){
				cout << "You have "<< rein <<" armies left to place\n";
				break;
			}
			cout << "You have "<< rein <<" armies left to place\n";
			string planet;
			int num;
			cin >> planet;
			cin >> num;
			if (cin.fail())
			{
				cout<<"Please enter a number for number of units.\n";
				cin.clear();
				cin.get(); //cleaning up cin is a pain...
				continue;
			}
			Planet* plan;
			bool isOwned=false;
			for(unsigned int i=0; i<plans.size(); i++)
				if(plans[i]->name()==planet)
				{
					isOwned=true;
					plan = plans[i];
					break;
				}
			if(!isOwned)
			{
				cout << "Please input a planet you own\n";
				continue;
			}
			if(num>rein)
			{
				cout << "Please input a number no larger than "<< rein << endl;
				continue;
			}
			Army* army = plan->armyHeld();
			army->reinforce(num);
			rein = rein - num;
		}
		cout << "All armies placed\n";
	}
}

void* flippies(void* arg){
  ALLEGRO_DISPLAY *display = (ALLEGRO_DISPLAY*)arg;
  al_set_target_backbuffer(display);
  ALLEGRO_COLOR red = al_map_rgb(202,72,10);
  ALLEGRO_COLOR green = al_map_rgb(17,105,17);
  ALLEGRO_COLOR white = al_map_rgb(255,255,255);
  ALLEGRO_COLOR brown = al_map_rgb(85,40,30);
  ALLEGRO_COLOR otherBrown = al_map_rgb(110,60,50);
  ALLEGRO_FONT *font = al_load_ttf_font("Assets/Xolonium-Regular.ttf", 16,0);
  const int scale = 20;
  char str[32];
  while(true){
    sleep(2);
    if(canFlip)
    {
        canFlip=false;
        vector<string> map = *altMap;
	vector<Planet*> planets = *altPlan;
	vector<Planet*> playerPlans = altCur->planetsHeld();
	vector<string> planetNames; //push all owned planets into here for coloration
	for(unsigned int i=0; i<playerPlans.size(); i++){
		planetNames.push_back(playerPlans[i]->name());
	}
	for(int i = 0; i < map.size(); ++i){
		for(int j = 0; j < map[i].size(); ++j){
			if(map[i][j] >= 65 && map[i][j] <= 90){
				string name = "";
				name.push_back(map[i][j]);
				al_draw_filled_circle(j*scale+10,i*scale+10,15,red);
				al_draw_text(font,white,j*scale+5,i*scale+5,0,name.c_str());
				for(int k = 0; k < planetNames.size(); ++k){
					if(planetNames[k] == name){
						al_draw_filled_circle(j*scale+10,i*scale+10,15,green);
						al_draw_text(font,white,j*scale+5,i*scale+5,0,name.c_str());
					}
				}
			}
			/*if(map[i][j] == '*'){
				int pickcol = rand()%2;
				if(pickcol){
					al_draw_filled_circle(j*scale+10,i*scale+10,4,brown);
				}else{
					al_draw_filled_circle(j*scale+10,i*scale+10,4,otherBrown);
				}
			}*/
		}
	}

	al_flip_display();
        canFlip=true;
    }
    sleep(2);
    if(canFlip){
        canFlip=false;
        vector<string> map = *altMap;
	vector<Planet*> planets = *altPlan;
	vector<Planet*> playerPlans = altCur->planetsHeld();
	vector<string> planetNames; //push all owned planets into here for coloration
	for(unsigned int i=0; i<playerPlans.size(); i++){
		planetNames.push_back(playerPlans[i]->name());
	}
	for(int i = 0; i < map.size(); ++i){
		for(int j = 0; j < map[i].size(); ++j){
			if(map[i][j] >= 65 && map[i][j] <= 90){
				string name = "";
				name.push_back(map[i][j]);
				al_draw_filled_circle(j*scale+10,i*scale+10,15,red);
				for(int k = 0; k < planetNames.size(); ++k)
					if(planetNames[k] == name)
						al_draw_filled_circle(j*scale+10,i*scale+10,15,green);
				for(int k = 0; k < planets.size(); ++k){
					if(planets[k]->name() == name){
						sprintf(str, "%d", planets[k]->armyHeld()->size());
						al_draw_text(font,white,j*scale,i*scale+2,0,str);
					}
				}
			}
		}
	}
	al_flip_display();
        canFlip=true;
    }
  }
}
