//Client (C/S)
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <iostream>
#include <sstream>

#define MAXPLAYERS 4
#define TILESIZE 100
#define N_TILES_WIDTH 8
#define N_TILES_HEIGHT 8

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
struct Player
{
	sf::Uint8 turn;
	std::string nick;
	sf::Vector2i position;
	sf::Text nameText;
	bool isDead = false;
	void moveTo(sf::Uint32 x, sf::Uint32 y);
};
enum Directions
{
	UP, DOWN, LEFT, RIGHT
};
namespace Game {
	enum gameState
	{
		others_turn,
		my_turn_chosing_move,
		my_turn_waiting_server
	};
	gameState currentState = gameState::others_turn;
	sf::Uint8 currentTurn = 0;
}

//Fordward declarations
void sendToServer(std::string mensaje);
void sendNickToServer(std::string nick);
void sendMove(int x, int y);
void recieveFromServer();
std::string statusToStr(sf::Socket::Status status);


//Global vars
sf::TcpSocket socket;		//Conexión con el servidor
std::string mensajeTeclado;
std::vector<Player> jugadores;
sf::Uint8 miTurno = 200;	//Se cambiará
sf::Text gameResult;

//Font
sf::Font font;

//Constants
const sf::Vector2i initialPositions[] = { sf::Vector2i(0,0),  sf::Vector2i(0,7),  sf::Vector2i(7,0),  sf::Vector2i(7,7)};

int main() {

	//Codi de la conexió al servidor 
	std::string user = "testing";
	std::cout << "Escribe tu nombre (sin espacios):\n";
	std::cin >> user;
	std::string ip = "";
	ip = "localhost";
	sf::Socket::Status st;
	st = socket.connect(ip, 50000, sf::milliseconds(15.f));
	if (st == sf::Socket::Error)
	{
		std::cout << "Error: No se ha podido conectar con el servidor\n";
		system("pause");
		return 0;
	}
	
	sendNickToServer(user);
	std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
	recieveFromServer();

	//Creación de la ventana
	sf::Vector2i screenDimensions(800, 900);
	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "TCPGame - "+user);
	//Para evitar que al mantener pulsado un botón se generen múltiples eventos
	window.setKeyRepeatEnabled(false);

	//Texturas, Sprites y fuentes
	sf::RectangleShape mapShape(sf::Vector2f(TILESIZE*N_TILES_WIDTH, TILESIZE*N_TILES_HEIGHT));
	//mapShape.setFillColor(sf::Color::Green);
	sf::Texture texture, characterTexture;
	if (!texture.loadFromFile("mapa2.png"))
		std::cout << "Error al cargar la textura del mapa!\n";
	if (!characterTexture.loadFromFile("personatgeTransp.png"))
		std::cout << "Error al cargar la textura del personaje!\n";
	if (!font.loadFromFile("courbd.ttf"))
		std::cout << "Error al cargar la fuente" << std::endl;

	mapShape.setTexture(&texture);	//s'hauria de provar amb sprites tmb
	sf::Sprite characterSprite = sf::Sprite(characterTexture);
	gameResult = sf::Text("", font, 50);

	socket.setBlocking(false);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			//Cerrar la ventana
			if (event.type == sf::Event::Closed)
				window.close();

			//Detectar eventos de teclado
			if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Escape))
				window.close();

			//Detectar eventos de ratón
			if (event.type == sf::Event::MouseButtonPressed) {
				std::cout << "Mouse Pressed at position: " << event.mouseButton.x << ":"
					<< event.mouseButton.y << std::endl;
				if (Game::currentTurn == miTurno)
				{
					sendMove(event.mouseButton.x, event.mouseButton.y);
				}
				//sf::Mouse::getPosition(window)
			}

			//Detectar si estamos escribiendo algo, enviar el texto si presionamos enter, borrar la ultima letra si apretamos Backspace
			if (event.type == sf::Event::TextEntered)
			{
				if (event.text.unicode > 31 && event.text.unicode < 128)
					mensajeTeclado.push_back(static_cast<char>(event.text.unicode));
			}
			if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Return)) {		//Si apretamos enter, se envia el mensaje que teniamos escrito - TODO: controlar que no s'envii si està buit
				if (!mensajeTeclado.empty()) {
					sendToServer(user+": "+mensajeTeclado);
					mensajeTeclado = "";
				}
			}
			if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::BackSpace)) {
				if (!mensajeTeclado.empty())
					mensajeTeclado.pop_back();
			}
		}

		recieveFromServer();


		switch (Game::currentState)
		{
			case Game::gameState::my_turn_chosing_move:
				break;
			default:
				break;
		}
		window.clear();

		//Dibujar el mapa i jugadores
		window.draw(mapShape);
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			characterSprite.setPosition(sf::Vector2f(jugadores.at(i).position*TILESIZE));
			if (jugadores.at(i).isDead)
			{
				characterSprite.rotate(-90);
				characterSprite.move(sf::Vector2f(0, TILESIZE));
			}
			window.draw(characterSprite);
			characterSprite.setRotation(0);
			window.draw(jugadores.at(i).nameText);
			window.draw(gameResult);
		}

		window.display();
	}

	socket.disconnect();

	return 0;
}

void sendNickToServer(std::string nick) {
	sf::Packet packet;
	packet << Comandos::mi_nick_es;
	packet << nick;

	sf::Socket::Status st;
	do
	{
		st = socket.send(packet);
	} while (st == sf::Socket::Partial);
	std::cout << "nick enviat amb status " << statusToStr(st) << std::endl;
}

void sendToServer(std::string mensaje) {
	sf::Packet packet;

	packet << Comandos::mensaje;

	packet << mensaje;

	sf::Socket::Status st;
	do
	{
		st = socket.send(packet);
	} while (st == sf::Socket::Partial);
	std::cout << "missatge enviat amb status " << statusToStr(st) << std::endl;
}

void sendMove(int x, int y) {
	if (x < N_TILES_WIDTH*TILESIZE && y < N_TILES_WIDTH*TILESIZE)
	{
		sf::Packet packet;
		packet << Comandos::Move;
		Directions dir;
		
		int dx = x - (jugadores.at(miTurno).position.x*TILESIZE + TILESIZE/2);
		int dy = y - (jugadores.at(miTurno).position.y*TILESIZE + TILESIZE/2);

		if (std::abs(dx) >= std::abs(dy))
		{
			if (dx >= 0)
				dir = Directions::RIGHT;
			else
				dir = Directions::LEFT;
		}
		else
		{
			if (dy >= 0)
				dir = Directions::DOWN;
			else
				dir = Directions::UP;
		}
		packet << dir;
		sf::Socket::Status st; 
		do
		{
			st = socket.send(packet);
		} while (st == sf::Socket::Partial);
		std::cout << "Move enviat amb status " << statusToStr(st) << std::endl;
	}
}


void recieveFromServer(){
	sf::Packet packet;
	sf::TcpSocket::Status result = socket.receive(packet);

	if (result == sf::TcpSocket::Status::Done)
	{
		int comandoInt;
		packet >> comandoInt;
		Comandos comando = (Comandos)comandoInt;

		switch (comando)
		{
		case inicio_partida:
		{
			std::string mensajeStr;
			std::stringstream ss;
			std::string playerN;
			packet >> miTurno;
			for (int i = 0; i < MAXPLAYERS; i++)
			{
				Player newPlayer;
				packet >> newPlayer.nick;
				newPlayer.turn = (sf::Uint8) i;
				newPlayer.position = initialPositions[i];
				newPlayer.nameText = sf::Text(newPlayer.nick, font, 14);
				newPlayer.nameText.setOutlineThickness(2.f);
				newPlayer.nameText.setOutlineColor(sf::Color(0, 0, 0));
				if (i==miTurno)
					newPlayer.nameText.setFillColor(sf::Color(0, 0, 250));
				else
					newPlayer.nameText.setFillColor(sf::Color(250, 0, 0));
				newPlayer.nameText.setStyle(sf::Text::Bold);
				newPlayer.nameText.setPosition(newPlayer.position.x*TILESIZE, newPlayer.position.y*TILESIZE);
				jugadores.push_back(newPlayer);
			}

			std::cout << "La partida va a empezar con los jugadores: "<< std::endl;
			for (int i = 0; i < jugadores.size(); i++)
			{
				std::cout << "->" << jugadores.at(i).nick << std::endl;
			}
			break;
		}
		case mensaje: 
		{
			std::string mensajeStr;
			packet >> mensajeStr;
			std::cout << mensajeStr << std::endl;
			break;
		}
		case Posicion_final: 
		{
			std::cout << "rebent move!" << std::endl;
			int dirInt;
			sf::Uint32 finalPosX = 0;
			sf::Uint32 finalPosY = 0;
			packet >> finalPosX;
			packet >> finalPosY;
			std::cout << "rebuda la posicio final (" << finalPosX << ":" << finalPosY << ")\n";
			jugadores.at((int)Game::currentTurn).moveTo(finalPosX, finalPosY);
		}
			break;
		case Es_Tu_Turno:
			packet >> Game::currentTurn;
			std::cout << "Es el turno de " << (int)Game::currentTurn << "(" << jugadores.at((int)Game::currentTurn).nick << ")" << std::endl;
			break;
		case Eliminado:
			sf::Uint8 eliminated;
			packet >> eliminated;
			if (eliminated == miTurno)
			{
				gameResult = sf::Text("Has sido eliminado!", font, 50);
				gameResult.setFillColor(sf::Color(250, 0, 0));
				gameResult.setPosition(100, 400);
			}
			jugadores.at((int)eliminated).isDead = true;
			std::cout << "El jugador " << (int)eliminated << " ha muerto!\n";
			break;
		case Ganador:
			sf::Uint8 ganador;
			packet >> ganador;
			if (miTurno == ganador)
			{
				gameResult = sf::Text("Has ganado la partida!", font, 50);
				gameResult.setFillColor(sf::Color(250, 0, 0));
				gameResult.setPosition(100, 400);
			}
			break;
		default:
			break;
		}
	}
	else if (result == sf::TcpSocket::Status::Disconnected) {
		socket.disconnect();
		std::cout << "El servidor se ha desconectado " << std::endl;
	}

}

std::string statusToStr(sf::Socket::Status status)
{
	switch (status)
	{
	case sf::Socket::Done:
		return "Done";
		break;
	case sf::Socket::NotReady:
		return "Not Ready";
		break;
	case sf::Socket::Partial:
		return "Partial";
		break;
	case sf::Socket::Disconnected:
		return "Disconnected";
		break;
	case sf::Socket::Error:
		return "Error";
		break;
	default:
		return "";
		break;
	}
	
}

void Player::moveTo(sf::Uint32 x, sf::Uint32 y) {
	position = sf::Vector2i(x, y);
	nameText.setPosition(x*TILESIZE, y*TILESIZE);
}