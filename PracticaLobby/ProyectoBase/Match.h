#pragma once
#include <SFML\Main.hpp>
#include <vector>
#include "ServerTypes.h"

#define MAX_PLAYERS 4
#define MAX_LEVEL_DIFFERENCE 4	//Número de niveles que otros jugadores pueden tener de más o menos al hacer macthmaking
#define N_TILES_WIDTH 8
#define N_TILES_HEIGHT 8
#define EMPTY 0
#define CHARACTER 1
#define DEAD 2

class Match
{
public:
	bool hasStarted;
	sf::Uint32 idGame;
	std::vector<Player> players;
	sf::Uint8 currentTurn = (sf::Uint8)0;
	int playersAlive = MAX_PLAYERS;
	bool gameFinished = false;
	Match();
	~Match();

	bool isLevelInRange(sf::Uint16 level);
	bool isFull();
	void sendMatchStart();
	void update();
	void sendCurrentTurnToAll();
	void playerDead(int x, int y);
	void sendWinner(sf::Uint8 winner);
	bool isOutsideMap(int x, int y);
	void changeTurn();
	void sendPacketToAll(sf::Packet &packet);
	void sendMoveToAll(Directions dir);
};

