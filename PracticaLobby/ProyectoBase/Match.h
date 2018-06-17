#pragma once
#include <SFML\Main.hpp>
#include <vector>
#include "ServerTypes.h"

#define MAX_PLAYERS 2
#define MAX_LEVEL_DIFFERENCE 3	//Número de niveles que otros jugadores pueden tener de más o menos al hacer macthmaking
#define SECONDS_UNTIL_KICK 4
#define N_TILES_WIDTH 8
#define N_TILES_HEIGHT 8
//MAP CELLS:
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
	sf::Clock timerUntilKick;

	int gameMap[8][8];

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
	void sendMessageToAll(std::string mes);
	//Sends a packet to all the players that are connected to that match. Disconnected players or palyers at other matches do not count.
	void sendPacketToAll(sf::Packet &packet);
	void sendMoveToAll(Directions dir);
	//Checks if there is only one player alive or only one player connected
	void checkIf1AliveOrConnected();
	//Kicks all players inside so the match can be destroyed (called when a match ends)
	void kickEveryone();
};

