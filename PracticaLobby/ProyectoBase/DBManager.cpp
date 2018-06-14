#pragma once
#include "DBManager.h"



DBManager::DBManager()
{
	driver = get_driver_instance();
	con = driver->connect(DB_HOST, USER, PWD);
	stmt = con->createStatement();
	stmt->execute("USE gamedb");
	std::cout << "Conexion con base de datos correcta" << std::endl;
}

bool DBManager::registerUser(std::string name, std::string pswd, std::string email)
{
	bool succes = false;

	try {
		//Comprovacions de si es pot registrar
		std::string queryString = std::string("select count(*) from Players where name='") + std::string(name) + std::string("'");
		resultSet = stmt->executeQuery(queryString.c_str());
		//sql::SQLString::SQLString(
		if (resultSet->next())
		{
			if (resultSet->getInt(1) == 0) {
				std::string insertString = std::string("INSERT INTO Players(name, password,email) VALUES('") + std::string(name) + std::string("', '") + std::string(pswd) + std::string("', '") + std::string(email) + std::string("')");

				stmt->execute(insertString.c_str());

				succes = true;
			}
		}
	} catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
	delete resultSet;

	return succes;
}

int DBManager::getLastRegistered() {
	try {
		std::string queryString = std::string("SELECT MAX(id) FROM Players");
		resultSet = stmt->executeQuery(queryString.c_str());
		if (resultSet->next())
		{
			return (int)resultSet->getInt(1);
		}
		else {
			return -1;
		}
		delete resultSet;
	} catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
	return -2;
}

bool DBManager::login(std::string name, std::string pswd, int &idPlayer, int &level)
{
	bool result = false;
	try {
		std::string queryString = std::string("SELECT id, level FROM Players WHERE name = '")+name+std::string("' AND password = '")+pswd+ std::string("'");
		resultSet = stmt->executeQuery(queryString.c_str());
		if (resultSet->next())
		{
			idPlayer = (int)resultSet->getInt(1);
			level = (int)resultSet->getInt(2);
			result = true;
		}
		else {
			idPlayer = -1;
		}
		delete resultSet;
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}

	return result;
}

void DBManager::addMatch(int idPlayer, int idSession)
{



}


DBManager::~DBManager()
{
	std::cout << "DELETE\n";
	/*delete stmt;
	delete con;*/
}

void saveSession(std::string name) {




}