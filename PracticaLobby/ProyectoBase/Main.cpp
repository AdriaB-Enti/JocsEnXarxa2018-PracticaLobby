//Proves DML-SQL
#pragma once

//#include <iostream>
#include <SFML\Network.hpp>
#include <string>
#include <map>
#include <vector>
#include "ServerTypes.h"

#include "DBManager.h"
#include "Match.h"

//fwd declarations
void recieveData();

std::map<sf::Uint64, Player> players;
std::map<sf::Uint32, Match> matches;
std::vector<Player> nonActivePlayers;
sf::Uint64 idMatchCounter;
DBManager dbm;

int main()
{
	std::cout << "first\n";

	sf::TcpListener listener;
	listener.setBlocking(false);
	listener.listen(50000);

	sf::Clock mathmakingWait;
	mathmakingWait.restart();
	while (true)
	{
		//Aceptar nuevas conexiones
		sf::TcpSocket* newSock = new sf::TcpSocket();
		sf::Socket::Status st = listener.accept(*newSock);
		if (st== sf::Socket::Status::Done)
		{
			std::string texto = "Conexion con ... " + (newSock->getRemoteAddress()).toString() + ":" + std::to_string(newSock->getRemotePort()) + "\n";
			std::cout << texto;

			Player newPlayer;
			newPlayer.playerSock = newSock;
			newPlayer.playerSock->setBlocking(false);
			nonActivePlayers.push_back(newPlayer);
		}


		recieveData();

		//Control non active players
		if (mathmakingWait.getElapsedTime().asMilliseconds() > 300)
		{
			for (auto playerIter = nonActivePlayers.begin(); playerIter != nonActivePlayers.end();)
			{
				if (playerIter->isLogged)
				{

					bool placed = false;
					for (auto matIter = matches.begin(); matIter != matches.end();)
					{
						if (!matIter->second.hasStarted && !matIter->second.isFull()) {
							if (matIter->second.isLevelInRange(MAX_LEVEL_DIFFERENCE))
							{
								std::cout << "jugador afegit a una existent\n";
								matIter->second.players.push_back(*playerIter);
								if (matIter->second.isFull()) {
									std::cout << "match full\n";
									matIter->second.hasStarted = true;
									matIter->second.sendMatchStart();
									matIter->second.sendCurrentTurnToAll();
								}
								placed = true;
								break;
							}
						}
						matIter++;
					}
					if (placed) {
						playerIter = nonActivePlayers.erase(playerIter);
					} else {
						std::cout << "jugador afegit a una nova\n";
						//crear match i afegir el jugador
						Match newMatch = Match();
						newMatch.players.push_back(*playerIter);
						std::cout << "current matchs: " << matches.size() << std::endl;
						matches.emplace(std::pair<sf::Uint32, Match>(idMatchCounter++, newMatch));
						playerIter = nonActivePlayers.erase(playerIter);
					}
				}
				else {
					playerIter++;
				}
			}
		}




	}
	


	system("pause");
	return 0;
}


void recieveData() {
	auto iter = nonActivePlayers.begin();
	while (iter != nonActivePlayers.end())
	{
		sf::Packet packet;
		sf::TcpSocket::Status result = iter->playerSock->receive(packet);

		if (result == sf::TcpSocket::Status::Done)
		{
			sf::Uint8 commandInt;
			packet >> commandInt;
			Comandos commandRecieved = (Comandos)commandInt;

			switch (commandRecieved)
			{
			case registro:
			{
				std::cout << "user registring\n";
				std::string name, pass, email;
				packet >> name;
				packet >> pass;
				packet >> email;

				int newPlayerId;
				if (dbm.registerUser(name, pass, email)) {
					newPlayerId = dbm.getLastRegistered();
					std::cout << "usuario registrado con la id " << newPlayerId << std::endl;
					iter->nick = name;
					iter->idPlayer = (sf::Uint64)newPlayerId;
					iter->level = 1;
					iter->isLogged = true;
				}
				else {
					newPlayerId = -1;
				}
				sf::Packet newPack;
				newPack << (sf::Uint8)Comandos::welcome;
				newPack << (sf::Uint64) newPlayerId;
				iter->playerSock->send(newPack);
			}
			break;
			case login:
			{
				std::cout << "user logging\n";
				std::string name, pass;
				packet >> name;
				packet >> pass;
				int playerId, playerLevel;
				if (dbm.login(name, pass,playerId, playerLevel)) {
					std::cout << "logging correct\n";
					sf::Packet newPack;
					newPack << (sf::Uint8)Comandos::welcome;
					newPack << (sf::Uint64) playerId;
					iter->playerSock->send(newPack);

					iter->nick = name;
					iter->idPlayer = (sf::Uint64)playerId;
					iter->level = (sf::Uint16)playerLevel;
					iter->isLogged = true;
					std::cout << "level: " << playerLevel << "\n";
				}
				else
					std::cout << "bad logging\n"; {
					sf::Packet newPack;
					newPack << (sf::Uint8)Comandos::Error;
					newPack << (sf::Uint8)Errors::login_error;
					iter->playerSock->send(newPack);
				}

			}
				break;
			case welcome:
				break;
			case mi_nick_es:
				break;
			case inicio_partida:
				break;
			case mensaje:
				break;
			case Move:
				break;
			case Posicion_final:
				break;
			case Eliminado:
				break;
			case Ganador:
				break;
			case Es_Tu_Turno:
				break;
			case buscar_partida:


				break;
			default:
				break;
			}
		}
		else if (result == sf::TcpSocket::Status::Disconnected)
		{
			std::cout << "Un cliente se ha desconectado" << std::endl;
			iter->playerSock->disconnect();
			//TODO: ESBORRAR
			iter->connected = false;
		}

		iter++;
	}

	//iterar jugadores en partidas (se hace dentro de match)
	for (auto match = matches.begin(); match != matches.end(); match++)
	{
		if (match->second.hasStarted)
		{
			match->second.update();
		}
	}
}