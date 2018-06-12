#pragma once
#include <string>

#include <stdlib.h>
#include <iostream>
#include <mysql_connection.h>
#include <cppconn\driver.h>
#include <cppconn\resultset.h>
#include <cppconn\statement.h>
#include <cppconn\exception.h>

//Constant values
#define DB_HOST "tcp://192.168.1.41:3306"
#define USER "root"
#define PWD "linux123"

class DBManager
{
public:
	DBManager();

	//Returns if resitrationa was succesful or not
	bool registerUser(std::string name, std::string pswd, std::string email);
	//Returns if login was succesful or not (name and password match)
	bool login(std::string name, std::string pswd, int &idPlayer, int &level);
	//Adds a match to the given session with the given player
	void addMatch(int idPlayer, int idSession);
	//Returns the last registered id
	int getLastRegistered();

	//Saves session from player with name 'name'
	void saveSession(std::string name);




	~DBManager();
private:
	sql::Driver* driver;
	sql::Connection* con;
	sql::Statement* stmt;
	sql::ResultSet* resultSet;
};

