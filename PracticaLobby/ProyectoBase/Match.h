#pragma once
#include <SFML\Main.hpp>
#include <vector>
#include "ServerTypes.h"

#define MAX_PLAYERS 4
#define MAX_LEVEL_DIFFERENCE 4	//N�mero de niveles que otros jugadores pueden tener de m�s o menos al hacer macthmaking


class Match
{
public:
	bool hasStarted;
	sf::Uint32 idGame;
	std::vector<Player> players;

	Match();
	~Match();

	bool isLevelInRange(sf::Uint16 level);
	bool isFull();
	void sendMatchStart();
};

