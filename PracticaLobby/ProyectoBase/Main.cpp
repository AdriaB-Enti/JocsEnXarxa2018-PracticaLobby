//Proves DML-SQL
#pragma once

#include <stdlib.h>
#include <iostream>
#include <mysql_connection.h>
#include <cppconn\driver.h>
#include <cppconn\resultset.h>
#include <cppconn\statement.h>
#include <cppconn\exception.h>

#define DB_HOST "tcp://192.168.122.38:3306"
#define USER "root"
#define PWD "linux123"

//user i pwd
//tenir un unic punt d'entrada -recieve

int main()
{
	sql::Driver* driver;
	sql::Connection* con;
	sql::Statement* stmt;
	sql::ResultSet* resultSet;

	driver = get_driver_instance();
	con = driver->connect(DB_HOST, USER, PWD);
	stmt = con->createStatement();
	stmt->execute("USE gamedb");

	stmt->execute("INSERT INTO Players(name, password, level) VALUES('Carmack','pass',5)");

	resultSet = stmt->executeQuery("select name from Players");
	while (resultSet->next())
	{
		std::cout << resultSet->getString(1).c_str() << std::endl;
	}
	delete resultSet;



	/*resultSet = stmt->executeQuery("select count(*) from Players where PlayerName='player1' and PlayerPassword='1234'");
	if (resultSet->next())
	{
		int num = resultSet->getInt(1);

		std::cout << "num resultados: " << num << std::endl;
	}
	delete resultSet;

	resultSet = stmt->executeQuery("select count(*) from Players where PlayerName='player1' and PlayerPassword='12345'");
	if (resultSet->next())
	{
		int num = resultSet->getInt(1);

		std::cout << "num resultados: " << num << std::endl;
	}
	delete resultSet;*/

	delete stmt;
	delete con;
	system("pause");
	return 0;
}