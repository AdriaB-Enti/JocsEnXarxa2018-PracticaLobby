#pragma once
#include <SFML\Main.hpp>
#include <vector>
#include "ServerTypes.h"
#define MAX_PLAYERS 4

class Match
{
public:
	bool hasStarted;
	//id
	std::vector<Player> players;

	Match();
	~Match();

	bool isLevelInRange(sf::Uint16 level);
	bool isFull();
};

