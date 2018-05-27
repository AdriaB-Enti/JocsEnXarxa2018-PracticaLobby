#pragma once
#include "Match.h"
#include <iostream>

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
		if (player->level > (level+ MAX_LEVEL_DIFFERENCE) || player->level < (level - MAX_LEVEL_DIFFERENCE))
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

void Match::sendMatchStart() {
	std::cout << "starting match " << (int)idGame << std::endl;
	//enviar start game a tots els players
	int turn = 0;

	for (auto player = players.begin(); player != players.end(); player++)
	{
		player->idGame = idGame;
		player->turn = turn;

		sf::Packet inicioPack;
		inicioPack << (sf::Uint8) Comandos::inicio_partida;
		inicioPack << (sf::Uint8) turn;
		for (auto p = players.begin(); p != players.end(); p++)
		{
			inicioPack << p->nick;
		}
		sf::Socket::Status st;
		do
		{
			st = player->playerSock->send(inicioPack);
		} while (st == sf::Socket::Status::Partial);

		turn++;
	}

	

}

Match::~Match()
{

}
