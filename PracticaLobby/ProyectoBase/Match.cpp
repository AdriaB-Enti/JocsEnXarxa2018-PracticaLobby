#include "Match.h"

Match::Match()
{
	hasStarted = false;
}

bool Match::isLevelInRange(sf::Uint16 level)
{
	bool inRange = true;

	//recorrer jugadores
	for (auto player = players.begin(); player != players.end(); player++)
	{
		if (player->level > level)
		{
			inRange = false;
		}
	}

	return inRange;
}

bool Match::isFull()
{
	return players.size() >= MAX_PLAYERS;
}



Match::~Match()
{

}
