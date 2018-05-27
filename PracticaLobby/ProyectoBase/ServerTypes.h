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

enum Directions
{
	UP, DOWN, LEFT, RIGHT
};

enum Comandos
{
	registro,
	login,
	welcome,
	mi_nick_es,
	inicio_partida,
	mensaje,
	Move,
	Posicion_final,
	Eliminado,
	Ganador,
	Es_Tu_Turno,
	Error,
	buscar_partida
};

enum Errors {
	login_error
};