#pragma once
#include <SFML\Network.hpp>

struct Player
{
	
	//socket
	sf::Uint32 idGame;
	sf::Uint64 idPlayer;
	sf::Uint8 turn;
	std::string nick;
	sf::Uint16 level;
	sf::Vector2i position;
	bool isLogged = false;
	bool isDead = false;
	bool connected = true;
	sf::TcpSocket* playerSock;
};