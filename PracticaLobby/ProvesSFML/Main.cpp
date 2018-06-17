//Client (C/S)
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <iostream>
#include <sstream>
#include <list>

#define MAXPLAYERS 2
#define TILESIZE 100
#define N_TILES_WIDTH 8
#define N_TILES_HEIGHT 8

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
	buscar_partida,
	desconectado,
	expulsado
};
enum Errors {
	login_error
};
struct Player
{
	sf::Uint8 turn;
	std::string nick;
	sf::Uint16 level;
	sf::Vector2i position;
	sf::Text nameText;
	bool isDead = false;
	bool disconnected = false;
	void moveTo(sf::Uint32 x, sf::Uint32 y);
};
enum Directions
{
	UP, DOWN, LEFT, RIGHT
};
namespace Game {
	sf::Uint8 currentTurn = 20;
}

namespace Screen {
	enum scene
	{
		searchingScene,
		playingScene,
		menuScene
	};
	scene currentScene = scene::searchingScene;
}

//Fordward declarations
void sendRegister(std::string nick, std::string pass, std::string email);
void sendLogin(std::string nick, std::string pass);
void waitForServerWelcome();



//game
void sendMessageToServer(std::string mensaje);
void sendMove(int x, int y);
void recieveFromServer();
std::string statusToStr(sf::Socket::Status status);


//Global vars
sf::TcpSocket socket;		//Conexión con el servidor
sf::Uint64 myId;
sf::Uint16 myLevel;
std::string mensajeTeclado;
std::vector<Player> jugadores;
sf::Uint8 miTurno = 200;	//Se cambiará
sf::Text gameResult, screenSubtitle, gameChat, writingMessage;
sf::Text matchmakingButtonText, exitButtonText;
std::list<std::string> lastMessages;	//The last 3 messages in chat;
sf::RenderWindow window;

//Font
sf::Font font;

//Constants
const sf::Vector2i initialPositions[] = { sf::Vector2i(0,0),  sf::Vector2i(0,7),  sf::Vector2i(7,0),  sf::Vector2i(7,7)};

int main() {

	//Inicio conexión al servidor 
	std::string ip = "";
	ip = "localhost";
	sf::Socket::Status st;
	st = socket.connect(ip, 50000, sf::milliseconds(2000.f));
	if (st == sf::Socket::Error)
	{
		std::cout << "Error: No se ha podido conectar con el servidor\n";
		system("pause");
		return 0;
	}
	socket.setBlocking(false);

	std::string answerStr, pass, user = "testing";
	std::cout << "Para poder jugar necesitas estar registrado.\nEres un usuario nuevo? [S/n]\n";		//Es necessari....?
	std::cin >> answerStr;
	bool registrado = true;
	if (answerStr.front() == 'S' || answerStr.front() == 's')
	{
		registrado = false;
	}
	
	std::cout << "Escribe tu nombre:\n";
	std::cin >> user;
	std::cout << "Escribe tu contraseña:\n";
	std::cin >> pass;

	//enviar user i pass
	if (registrado)
	{
		sendLogin(user, pass);
	}
	else {
		std::string email;
		std::cout << "Escribe tu email:\n";
		std::cin >> email;
		sendRegister(user, pass, email);
	}

	waitForServerWelcome();

	sf::Packet matchm;
	matchm << (sf::Uint8)Comandos::buscar_partida;
	matchm << myId;
	socket.send(matchm);


	//sendNickToServer(user);
	std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
	recieveFromServer();

	//Creación de la ventana
	sf::Vector2i screenDimensions(800, 900);
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "TCPGame - "+user);
	//Para evitar que al mantener pulsado un botón se generen múltiples eventos
	window.setKeyRepeatEnabled(false);

	//Texturas, Sprites y fuentes
	sf::RectangleShape mapShape(sf::Vector2f(TILESIZE*N_TILES_WIDTH, TILESIZE*N_TILES_HEIGHT));
	sf::RectangleShape lobbyShape = mapShape;
	//mapShape.setFillColor(sf::Color::Green);
	sf::Texture texture, characterTexture, lobbyTexture, buttonTexture;
	if (!lobbyTexture.loadFromFile("greytext.png"))
		std::cout << "Error al cargar la textura del lobby!\n";
	if (!texture.loadFromFile("mapa2.png"))
		std::cout << "Error al cargar la textura del mapa!\n";
	if (!characterTexture.loadFromFile("personatgeTransp.png"))
		std::cout << "Error al cargar la textura del personaje!\n";
	if (!buttonTexture.loadFromFile("button.png"))
		std::cout << "Error al cargar la textura del boton!\n";
	if (!font.loadFromFile("courbd.ttf"))
		std::cout << "Error al cargar la fuente" << std::endl;


	mapShape.setTexture(&texture);
	lobbyShape.setTexture(&lobbyTexture);
	sf::Sprite characterSprite = sf::Sprite(characterTexture);
	sf::Sprite buttonSprite = sf::Sprite(buttonTexture);

	gameResult = sf::Text("BUSCANDO PARTIDA", font, 50);
	gameResult.setPosition(window.getSize().x / 5, 100);
	screenSubtitle = sf::Text("Con jugadores con nivel similar ("+ std::to_string(myLevel)+")", font, 20);
	screenSubtitle.setPosition(window.getSize().x / 4, 175);
	screenSubtitle.setFillColor(sf::Color::White);
	gameChat = sf::Text("", font, 18);
	gameChat.setPosition(sf::Vector2f(15, window.getSize().y - 95));
	writingMessage = sf::Text("", font, 18);
	writingMessage.setPosition(sf::Vector2f(15, window.getSize().y - 30));
	writingMessage.setFillColor(sf::Color::Yellow);
	writingMessage.setString("(Chat desabilitado fuera de partidas)");

	matchmakingButtonText = sf::Text("Matchmaking", font, 25);
	matchmakingButtonText.setPosition(66 + window.getSize().x / 2 - buttonTexture.getSize().x / 2, 36 + 3 * window.getSize().y / 8);
	matchmakingButtonText.setFillColor(sf::Color::Green);

	exitButtonText = sf::Text("Salir", font, 25);
	exitButtonText.setPosition(105 + window.getSize().x / 2 - buttonTexture.getSize().x / 2, 36 + 5 * window.getSize().y / 8);
	exitButtonText.setFillColor(sf::Color::Green);

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
				if (Screen::currentScene == Screen::scene::playingScene) {
					if (Game::currentTurn == miTurno)
						sendMove(event.mouseButton.x, event.mouseButton.y);
				}
				if (Screen::currentScene == Screen::scene::menuScene)
				{
					//Matchmaking button
					if (event.mouseButton.x > buttonSprite.getPosition().x && event.mouseButton.x < buttonSprite.getPosition().x+buttonTexture.getSize().x
						&& event.mouseButton.y > 3 * window.getSize().y / 8 && event.mouseButton.y < (3 * window.getSize().y / 8) + buttonTexture.getSize().y)
					{
						sf::Packet match_packet;
						match_packet << (sf::Uint8)Comandos::buscar_partida;
						sf::Socket::Status st;
						do
						{
							st = socket.send(match_packet);
						} while (st == sf::Socket::Partial);
						Screen::currentScene == Screen::scene::searchingScene;
						gameResult.setString("");
					}
					//Exit button
					if (event.mouseButton.x > buttonSprite.getPosition().x && event.mouseButton.x < buttonSprite.getPosition().x + buttonTexture.getSize().x
						&& event.mouseButton.y > 5 * window.getSize().y / 8 && event.mouseButton.y < (5 * window.getSize().y / 8) + buttonTexture.getSize().y)
					{
						window.close();
					}
				}
				//sf::Mouse::getPosition(window)
			}

			//Solo podemos escribir si estamos en una partida
			if (Screen::currentScene == Screen::scene::playingScene)
			{
				//Detectar si estamos escribiendo algo, enviar el texto si presionamos enter, borrar la ultima letra si apretamos Backspace
				if (event.type == sf::Event::TextEntered)
				{
					if (event.text.unicode > 31 && event.text.unicode < 128) {
						mensajeTeclado.push_back(static_cast<char>(event.text.unicode));
						writingMessage.setString("Escribe: " + mensajeTeclado);
					}
				}
				if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Return)) {		//Si apretamos enter, se envia el mensaje que teniamos escrito
					if (!mensajeTeclado.empty()) {
						sendMessageToServer(user+": "+mensajeTeclado);
						mensajeTeclado = "";
						writingMessage.setString("Escribe: " + mensajeTeclado);
					}
				}
				if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::BackSpace)) {
					if (!mensajeTeclado.empty()) {
						mensajeTeclado.pop_back();
						writingMessage.setString("Escribe: " + mensajeTeclado);
					}

				}
			}

		}

		recieveFromServer();


		window.clear();

		if (Screen::currentScene == Screen::scene::searchingScene)
		{
			window.draw(lobbyShape);
			window.draw(screenSubtitle);
		}else if (Screen::currentScene == Screen::scene::playingScene) {
			//Dibujar el mapa i jugadores
			window.draw(mapShape);
			for (auto player = jugadores.begin(); player != jugadores.end(); player++)
			{
				if (!player->disconnected)
				{
					characterSprite.setPosition(sf::Vector2f(player->position*TILESIZE));
					if (player->isDead)
					{
						characterSprite.rotate(-90);
						characterSprite.move(sf::Vector2f(0, TILESIZE));
					}
					window.draw(characterSprite);
					characterSprite.setRotation(0);
					window.draw(player->nameText);
				}
			}
		} else if (Screen::currentScene == Screen::scene::menuScene) {
			window.draw(lobbyShape);
			//Menu buttons
			buttonSprite.setPosition(window.getSize().x / 2 - buttonTexture.getSize().x / 2, 3 * window.getSize().y / 8);
			window.draw(buttonSprite);
			window.draw(matchmakingButtonText);

			buttonSprite.setPosition(window.getSize().x / 2 - buttonTexture.getSize().x / 2, 5 * window.getSize().y / 8);
			window.draw(buttonSprite);
			window.draw(exitButtonText);
		}

		window.draw(gameResult);
		window.draw(gameChat);
		window.draw(writingMessage);
		window.display();
	}

	socket.disconnect();

	return 0;
}



void sendRegister(std::string nick, std::string pass, std::string email){
	sf::Packet packet;
	packet << (sf::Uint8)Comandos::registro;
	packet << nick;
	packet << pass;
	packet << email;
	sf::Socket::Status st;
	do
	{
		st = socket.send(packet);
	} while (st == sf::Socket::Partial);

}
void sendLogin(std::string nick, std::string pass) {
	sf::Packet packet;
	packet << (sf::Uint8)Comandos::login;
	packet << nick;
	packet << pass;
	sf::Socket::Status st;
	do
	{
		st = socket.send(packet);
	} while (st == sf::Socket::Partial);
}

void waitForServerWelcome() {

	sf::Packet packResponse;
	sf::Socket::Status stResponse;

	std::cout << "Esperando la respuesta del servidor...\n";
	stResponse = socket.receive(packResponse);
	while (stResponse != sf::TcpSocket::Status::Done)
	{
		sf::sleep(sf::milliseconds(200.f)); 
		stResponse = socket.receive(packResponse);
	}

	sf::Uint8 comandoInt;
	packResponse >> comandoInt;
	Comandos comando = (Comandos)comandoInt;
	if (comando == Comandos::welcome)
	{
		packResponse >> myId;
		packResponse >> myLevel;
		std::cout << "Bienvenido al servidor, usuario " << (int)myId << " (lvl " << (int)myLevel << ")\n";
	}
	else if (comando == Comandos::Error) {
		sf::Uint8 errorInt;
		packResponse >> errorInt;
		if ((Errors)errorInt == Errors::login_error)
		{
			std::cout << "Error: el nombre de usuario i/o contraseña son incorrectos\n";
			system("pause");
			exit(0);
		}
	}

}

void sendMessageToServer(std::string mensaje) {
	sf::Packet packet;

	packet << (sf::Uint8) Comandos::mensaje;

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
		packet << (sf::Uint8) Comandos::Move;
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
		std::cout << "pack recieved\n";
		sf::Uint8 comandoInt;
		packet >> comandoInt;
		Comandos comando = (Comandos)comandoInt;

		switch (comando)
		{
		case inicio_partida:
		{
			std::cout << "Game started\n";
			Screen::currentScene = Screen::scene::playingScene;
			writingMessage.setString("Escribe: ");
			gameResult.setString("");

			std::string mensajeStr;
			std::stringstream ss;
			std::string playerN;
			packet >> miTurno;
			for (int i = 0; i < MAXPLAYERS; i++)
			{
				Player newPlayer;
				packet >> newPlayer.nick;
				packet >> newPlayer.level;
				newPlayer.turn = (sf::Uint8) i;
				newPlayer.position = initialPositions[i];
				std::string playerNameLabel = newPlayer.nick + "-lvl" + std::to_string(newPlayer.level);
				newPlayer.nameText = sf::Text(playerNameLabel, font, 14);
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
			//Update in-game screen messages list (3 lines at the same time max)
			lastMessages.push_front(mensajeStr);
			while (lastMessages.size() > 3)
			{
				lastMessages.pop_back();
			}
			std::string allMessages;
			for (auto i = lastMessages.rbegin(); i != lastMessages.rend(); i++)
			{
				allMessages.append(*i + "\n");
			}
			gameChat.setString(allMessages);
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
			//jugadores.at((int)eliminated)
			std::cout << "El jugador " << (int)eliminated << " ha muerto!\n";
			break;
		case desconectado:
			sf::Uint8 disconnected;
			packet >> disconnected;
			jugadores.at((int)disconnected).disconnected = true;
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
		case expulsado:
			Screen::currentScene = Screen::scene::menuScene;
			gameResult = sf::Text("Menu principal", font, 50);
			gameResult.setFillColor(sf::Color(250, 255, 255));
			gameResult.setPosition(window.getSize().x / 5, 40);
			writingMessage.setString("(Chat desabilitado fuera de partidas)");
			break;
		default:
			break;
		}
	}
	else if (result == sf::TcpSocket::Status::Disconnected) {
		socket.disconnect();
		std::cout << "El servidor se ha desconectado " << std::endl;
		if (window.isOpen())
		{
			window.close();
		}
		return;
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