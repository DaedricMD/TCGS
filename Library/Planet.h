//Matthew Dyer, Bryan Studabaker, Zachary Baldwin
//Planet Object

#include <iostream>
#include <vector>
#include <string>
#include "army.h"

using namespace std;

#ifndef PLANET__H
#define PLANET__H

//class for the Planet object
class Planet{
	public:
		Planet();
		Planet(string pName, Army* armies,string owner, vector<string> connections); //constructor
		string name(); //returns the name of the planet as a string
		int armiesHeld(); // returns the number of armies on a planet as an integer
		Army* armyHeld(); //returns Army held
		string whoOwnsPlanet(); // returns the owner of a planet as a string
		vector<string> allConnections(); // returns a vector of all connections
		bool isConnected(Planet x); // returns true if planet x is connected to planet

		Army* army;
		void setOwner(string newName);
	private:
		string planetName;
		//int numArmies;
		string planetOwner;
		vector<string> planetConnections;
};

#endif
