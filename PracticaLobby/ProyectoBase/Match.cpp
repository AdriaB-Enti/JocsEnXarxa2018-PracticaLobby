#pragma once
#include "Match.h"
#include <iostream>

int gameMap[8][8] = {
	{ 0,0,0,0,0,0,0,0 },
{ 0,0,0,0,0,0,0,0 },
{ 0,0,0,0,0,0,0,0 },
{ 0,0,0,0,0,0,0,0 },
{ 0,0,0,0,0,0,0,0 },
{ 0,0,0,0,0,0,0,0 },
{ 0,0,0,0,0,0,0,0 },
{ 0,0,0,0,0,0,0,0 },
};


Match::Match()
{
	hasStarted = false;
	currentTurn = (sf::Uint8)0;
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
			inicioPack << p->level;
			//TODO: enviar el nivell
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

void Match::sendCurrentTurnToAll()
{
	sf::Packet turnPacket;
	turnPacket << (sf::Uint8) Comandos::Es_Tu_Turno;
	turnPacket << currentTurn;
	sendPacketToAll(turnPacket);
}

void Match::playerDead(int x, int y)
{
	playersAlive--;
	gameMap[x][y] = DEAD;
	sf::Packet pack;
	pack << (sf::Uint8) Comandos::Eliminado;
	for (int p = 0; p < MAX_PLAYERS; p++) {
		if (players.at(p).position.x == x && players.at(p).position.y == y)
		{
			players.at(p).isDead = true;
			pack << players.at(p).turn;
			//sendMsgToAll("Servidor: El jugador " + players.at(p).nick + " ha muerto!");
			break;
		}
	}
	std::cout << "Ha muerto un jugador!" << std::endl;
	sendPacketToAll(pack);

}

void Match::sendWinner(sf::Uint8 winner) //Uint8?????
{
	sf::Packet pack;
	pack << (sf::Uint8) Comandos::Ganador;
	pack << winner;
	sendPacketToAll(pack);
}

bool Match::isOutsideMap(int x, int y)
{
	if (x < 0 || x >= N_TILES_WIDTH || y < 0 || y >= N_TILES_HEIGHT)
	{
		return true;
	}
	return false;
}

void Match::changeTurn()
{
	do
	{
		currentTurn = (sf::Uint8) (++currentTurn % MAX_PLAYERS);
	} while (players.at(currentTurn).isDead || !players.at(currentTurn).connected);
	std::cout << "Changing turn to" << (int)currentTurn << "and sending..." << std::endl;
	sendCurrentTurnToAll();
}

void Match::sendPacketToAll(sf::Packet & packet)
{
	int jug = 0;
	for (auto itera = players.begin(); itera != players.end(); ++itera)
	{
		if (players.at(jug).connected)
		{
			sf::Socket::Status st;
			do
			{
				st = (*itera).playerSock->send(packet);
												//std::cout << "sending packet result: " << st << std::endl;
			} while (st == sf::Socket::Partial);
			if (st == sf::Socket::Disconnected)
			{
				players.at(jug).connected = false;
				(*itera).playerSock->disconnect();
				//sendMsgToAll("Se ha desconectado el jugador "+ players.at(jug).nick);
				//TODO: enviar commando disconnect
				//liberar memoria
			}
		}
		jug++;
	}

}

void Match::sendMoveToAll(Directions dir)
{
	Player * movingPlayer = &(players.at(currentTurn));

	sf::Int32 changingX = 0;
	sf::Int32 changingY = 0;
	//gameMap
	if (dir == Directions::RIGHT)
		changingX = 1;
	if (dir == Directions::LEFT)
		changingX = -1;
	if (dir == Directions::DOWN)
		changingY = 1;
	if (dir == Directions::UP)
		changingY = -1;

	sf::Int32 finalPosX = movingPlayer->position.x;
	sf::Int32 finalPosY = movingPlayer->position.y;

	bool positionFound = false;
	while (!positionFound)
	{
		if (isOutsideMap(finalPosX + changingX, finalPosY + changingY))
		{
			positionFound = true;
		}
		else {
			if (gameMap[finalPosX + changingX][finalPosY + changingY] == CHARACTER)
			{
				positionFound = true;
				playerDead(finalPosX + changingX, finalPosY + changingY);
			}
			else if (gameMap[finalPosX + changingX][finalPosY + changingY] == DEAD)
			{
				positionFound = true;
			}
			else
			{
				finalPosX += changingX;
				finalPosY += changingY;
			}
		}
	}

	//Atualitzar gameMap i posición jugador
	gameMap[movingPlayer->position.x][movingPlayer->position.y] = EMPTY;
	gameMap[finalPosX][finalPosY] = CHARACTER;
	movingPlayer->position.x = finalPosX;
	movingPlayer->position.y = finalPosY;

	sf::Packet pack;
	pack << (sf::Uint8) Comandos::Posicion_final;
	pack << finalPosX;
	pack << finalPosY;
	std::cout << "moving player to: " << finalPosX << ":" << finalPosY << std::endl;
	sendPacketToAll(pack);

}

void Match::update() {


	auto it = players.begin();

	int njugador = 0;
	while (it != players.end())			//fem un recieve per cada client
	{
		if (players.at(njugador).connected)
		{
			sf::Packet packet;
			sf::TcpSocket::Status result = (*it).playerSock->receive(packet);

			if (result == sf::TcpSocket::Status::Done)
			{
				sf::Uint8 commandInt;
				packet >> commandInt;
				Comandos commandRecieved = (Comandos)commandInt;

				switch (commandRecieved)
				{
					case mensaje: 
					{
						std::string newMessage;
						packet >> newMessage;

						//Reenviar el mensaje a todos los jugadores
						sf::Packet messagePacket;
						messagePacket << (sf::Uint8) Comandos::mensaje;
						messagePacket << newMessage;
						sendPacketToAll(messagePacket);
					}
						break;
					case Move:
					{
						std::cout << "rebent un move de " << njugador << std::endl;
						//comprovar que es el seu turn
						if (currentTurn == players.at(njugador).turn) {
							int direction;
							packet >> direction;
							sendMoveToAll((Directions)direction);
							if (playersAlive == 1 && !gameFinished)
							{
								sendWinner(currentTurn);
								gameFinished = true;
							}
							//de ser veritat -> Comandos::Ganador
							//del contrari, canviar el turn
							if (!gameFinished)
								changeTurn();
						}
						break;
					}
					default:
						break;
				}
				//++it;
			}
			else if (result == sf::TcpSocket::Status::Disconnected)
			{
				std::cout << "Un cliente se ha desconectado" << std::endl;
				(*it).playerSock->disconnect();
				players.at(njugador).connected = false;
				playersAlive--;
				sf::Packet disconPacket;
				disconPacket << (sf::Uint8) Comandos::mensaje;
				disconPacket << std::string("Servidor: el jugador "+ players.at(njugador).nick+" se ha desconectado");
				sendPacketToAll(disconPacket);
				//TODO: enviar comando disconect a tots els altres
				//--delete * it;
				//it = sockets.erase(it);
				//++it;
			}
			else if (result == sf::TcpSocket::Status::Error) {
				std::cout << "ERROR!!!" << std::endl;
				//++it;
			}
			else if (result == sf::TcpSocket::Status::NotReady) {
				//std::cout << "NOT READY---------" << std::endl;
				//++it;
			}
			else
			{
				//++it;
			}

		}
		++it;
		njugador++;
	}



}
