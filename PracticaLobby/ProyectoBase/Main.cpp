//Servidor (C/S)
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>

#define MAXPLAYERS 4
#define N_TILES_WIDTH 8
#define N_TILES_HEIGHT 8
#define EMPTY 0
#define CHARACTER 1
#define DEAD 2

enum Comandos
{
	mi_nick_es,
	inicio_partida,
	mensaje,
	Move,
	Posicion_final,
	Eliminado,
	Ganador,
	Es_Tu_Turno
};
enum Directions
{
	UP, DOWN, LEFT, RIGHT
};
struct Player
{
	//socket
	sf::Uint8 turn;
	std::string nick;
	sf::Vector2i position;
	bool isDead = false;
	bool connected = true;
};


//Fordward declarations
void sendMsgToAll(std::string mensajeStr);		//envia un mensaje a todos los clientes conectados
void sendNicksToAll();							//envia todos los nicks
void sendMoveToAll(Directions dir);
void sendPacketToAll(sf::Packet &packet);
void sendCurrentTurnToAll();
void sendWinner(sf::Uint8 winner);
void changeTurn();
bool isOutsideMap(int x, int y);
void playerDead(int x, int y);

//Global vars
std::vector<sf::TcpSocket*> sockets;
std::vector<Player> players;
bool waitingForMove = true;
sf::Uint8 currentTurn = (sf::Uint8)0;
int playersAlive = MAXPLAYERS;
bool gameFinished = false;

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

//Starting Positions
sf::Vector2i initialPositions[] = { sf::Vector2i(0,0),  sf::Vector2i(0,7),  sf::Vector2i(7,0),  sf::Vector2i(7,7) };

int main()
{
	sf::TcpListener listener;
	listener.listen(50000);
	for (int playerN = 0; playerN < MAXPLAYERS; playerN++)
	{
		sf::TcpSocket* newSock = new sf::TcpSocket();
		listener.accept(*newSock);
		sockets.push_back(newSock);

		std::string texto = "Conexion con ... " + (newSock->getRemoteAddress()).toString() + ":" + std::to_string(newSock->getRemotePort()) + "\n";
		std::cout << texto;

		Player newPlayer;
		newPlayer.turn = playerN;
		sf::Packet packetNick;
		newSock->receive(packetNick);
		int com;
		packetNick >> com;						//Preguntar suposo que no cal controlar que el comando sigui un nick, perque sempre serà el primer missatge......
		packetNick >> newPlayer.nick;
		newPlayer.position = initialPositions[playerN];
		gameMap[newPlayer.position.x][newPlayer.position.y] = CHARACTER;
		players.push_back(newPlayer);

		//sendMsgToAll(sockets, "Se ha conectado un nuevo usuario. Bienvenido!"); //Enviamos un mensaje a los demás clientes cuando alguien se conecta
		newSock->setBlocking(false);
	}
	listener.close();

	sendNicksToAll();

	std::cout << "Todos los usuarions conectados. El chat se ha iniciado!\n";
	sendMsgToAll("Todos los usuarios conectados");
	
	sendCurrentTurnToAll();

	while (true)																							//el server estarà sempre obert
	{
		std::vector<sf::TcpSocket*>::iterator it = sockets.begin();

		int njugador = 0;	//TODO: iterar des de jugadores, que els sockets estiguin allà
		while (it != sockets.end())			//fem un recieve per cada client
		{
			if (players.at(njugador).connected)
			{
				//sf::TcpSocket::Status result = (*it)->receive(buffer, 100, bytesReceived);
				sf::Packet packet;
				sf::TcpSocket::Status result = (*it)->receive(packet);

				//buffer[bytesReceived] = '\0';
				if (result == sf::TcpSocket::Status::Done)
				{
					int commandInt;
					packet >> commandInt;
					Comandos commandRecieved = (Comandos) commandInt;

					std::string lastMessage;
					switch (commandRecieved)
					{
						case mensaje:
							packet >> lastMessage;
							sendMsgToAll(lastMessage);
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
					(*it)->disconnect();
					players.at(njugador).connected = false;
					playersAlive--;
					//sendMsgToAll("Se ha desconectado el jugador " + players.at(njugador).nick);
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

	system("pause");

	return 0;

}

void sendNicksToAll() {
	sf::Uint8 turno = 0;
	for (std::vector<sf::TcpSocket*>::iterator itera = sockets.begin(); itera != sockets.end(); ++itera)
	{
		sf::Packet packet;
		packet << Comandos::inicio_partida;
		packet << turno;
		for (int p = 0; p < MAXPLAYERS; p++)
		{
			packet << players.at(p).nick;
		}
		sf::Socket::Status st;
		do
		{
			st = (*itera)->send(packet);
		} while (st == sf::Socket::Status::Partial);
		//std::cout << "sent result " << st << std::endl;
		turno++;
	}
}

//Sends the message to everyone except the same person
void sendMsgToAll(std::string mensajeStr) {
	sf::Packet packet;
	packet << Comandos::mensaje;
	packet << mensajeStr;
	sendPacketToAll(packet);
}

//From the direction given, calculates the final position and sends it to everyone
void sendMoveToAll(Directions dir) {
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
	pack << Comandos::Posicion_final;
	pack << finalPosX;
	pack << finalPosY;
	std::cout << "moving player to: " << finalPosX << ":" << finalPosY << std::endl;
	sendPacketToAll(pack);
}
void sendPacketToAll(sf::Packet &packet) {
	int jug = 0;
	for (std::vector<sf::TcpSocket*>::iterator itera = sockets.begin(); itera != sockets.end(); ++itera)
	{
		if (players.at(jug).connected)
		{
			sf::Socket::Status st;
			do
			{
				st = (*itera)->send(packet);	//s'ha de controlar els partials quan envies packets?????
				//std::cout << "sending packet result: " << st << std::endl;
			} while (st == sf::Socket::Partial);
			if (st == sf::Socket::Disconnected)
			{
				players.at(jug).connected = false;
				(*itera)->disconnect();
				//sendMsgToAll("Se ha desconectado el jugador "+ players.at(jug).nick);
				//liberar memoria
			}
		}
		jug++;
	}
}

void changeTurn()
{
	do
	{
		currentTurn = (sf::Uint8) (++currentTurn % MAXPLAYERS);
	} while (players.at(currentTurn).isDead || !players.at(currentTurn).connected);
	std::cout << "Changing turn to" << (int)currentTurn <<"and sending..." << std::endl;
	sendCurrentTurnToAll();
}


void sendCurrentTurnToAll()
{
	sf::Packet turnPacket;
	turnPacket << Comandos::Es_Tu_Turno;
	turnPacket << currentTurn;
	sendPacketToAll(turnPacket);
}

bool isOutsideMap(int x, int y) {
	if (x < 0 || x >= N_TILES_WIDTH || y < 0 || y >= N_TILES_HEIGHT)
	{
		return true;
	}
	return false;
}

//Checks who has died and sends it to everyone
void playerDead(int x, int y) {
	playersAlive--;
	gameMap[x][y] = DEAD;
	sf::Packet pack;
	pack << Comandos::Eliminado;
	for (int p = 0; p < MAXPLAYERS; p++) {
		if (players.at(p).position.x == x && players.at(p).position.y == y)
		{
			players.at(p).isDead = true;
			pack << players.at(p).turn;
			sendMsgToAll("Servidor: El jugador " + players.at(p).nick+ " ha muerto!");
			break;
		}
	}
	std::cout << "Ha muerto un jugador!" << std::endl;
	sendPacketToAll(pack);
}

void sendWinner(sf::Uint8 winner) {
	sf::Packet pack;
	pack << Comandos::Ganador;
	pack << winner;
	sendPacketToAll(pack);
}