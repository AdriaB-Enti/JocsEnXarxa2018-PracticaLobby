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
std::vector<Player> nonActivePlayers, searchingPlayers;
sf::Uint64 idMatchCounter;
DBManager dbm;

int main()
{
	sf::TcpListener listener;
	listener.setBlocking(false);
	listener.listen(50000);

	sf::Clock mathmakingWait;
	mathmakingWait.restart();

	std::cout << "Servidor encendido y funcionando\n";

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
			searchingPlayers.push_back(newPlayer);
		}


		recieveData();

		//Control non active players
		if (mathmakingWait.getElapsedTime().asMilliseconds() > 300)
		{
			for (auto playerIter = searchingPlayers.begin(); playerIter != searchingPlayers.end();)
			{
				if (playerIter->isLogged)
				{

					bool placed = false;
					for (auto matIter = matches.begin(); matIter != matches.end();)
					{
						if (!matIter->second.hasStarted && !matIter->second.isFull()) {
							if (matIter->second.isLevelInRange(playerIter->level))
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
						playerIter = searchingPlayers.erase(playerIter);
					} else {
						std::cout << "jugador afegit a una nova\n";
						//crear match i afegir el jugador
						Match newMatch = Match();
						newMatch.players.push_back(*playerIter);
						std::cout << "current matchs: " << matches.size() << std::endl;
						matches.emplace(std::pair<sf::Uint32, Match>(idMatchCounter++, newMatch));
						playerIter = searchingPlayers.erase(playerIter);
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

//Recieve data from players who aren't inside a match
void recieveData() {
	auto iter = searchingPlayers.begin();
	while (iter != searchingPlayers.end())
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


				//TODO: apuntar en el log de conexions
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

					//TODO: apuntar en el log de conexions
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

	//Actualizar todas las partidas (incluye iterar sobre los jugadores que hay en ellas)
	for (auto match = matches.begin(); match != matches.end(); )
	{
		if (match->second.hasStarted)
		{
			match->second.update();
		}

		if (match->second.gameFinished || match->second.playersAlive <= 0)
		{
		//	TODO: Si se ha acabado la partida, activar contador atrás
			if (match->second.playersAlive <= 0 || match->second.timerUntilKick.getElapsedTime().asSeconds() > SECONDS_UNTIL_KICK )
			{
				std::cout << "Eliminando partida finalizada con id= " << match->second.idGame << std::endl;
				match->second.kickEveryone();
				match = matches.erase(match);
			}
		} else
		{
			match++;
		}
		
	}
}