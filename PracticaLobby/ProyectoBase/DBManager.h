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
#define DB_HOST "tcp://192.168.122.38:3306"
#define USER "root"
#define PWD "linux123"

class DBManager
{
public:
	DBManager();

	//Returns if resitrationa was succesful or not
	bool registerUser(std::string name, std::string pswd);
	//Returns if login was succesful or not (name and password match)
	bool login(std::string name, std::string pswd);
	//Adds a match to the given session with the given player
	void addMatch(int idPlayer, int idSession);

	~DBManager();
private:
	sql::Driver* driver;
	sql::Connection* con;
	sql::Statement* stmt;
	sql::ResultSet* resultSet;
};

