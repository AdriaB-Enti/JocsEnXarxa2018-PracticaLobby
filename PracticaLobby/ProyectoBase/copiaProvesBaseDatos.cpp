
/*
#pragma once

#include <stdlib.h>
#include <iostream>
#include <mysql_connection.h>
#include <cppconn\driver.h>
#include <cppconn\resultset.h>
#include <cppconn\statement.h>
#include <cppconn\exception.h>

int main()
{
	sql::Driver* driver;
	sql::Connection* con;
	sql::Statement* stmt;
	sql::ResultSet* resultSet;

	driver = get_driver_instance();
	con = driver->connect("tcp://localhost:3306", "root", "");
	stmt = con->createStatement();
	stmt->execute("USE gamedb");


	resultSet = stmt->executeQuery("select PlayerName, PlayerPassword from Players");
	while (resultSet->next())
	{
		std::cout << resultSet->getString("PlayerName").c_str() << " - " <<
			resultSet->getString("PlayerPassword").c_str() << std::endl;
	}
	delete resultSet;

	resultSet = stmt->executeQuery("select count(*) from Players where PlayerName='player1' and PlayerPassword='1234'");
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
	delete resultSet;

	delete stmt;
	delete con;
	system("pause");
	return 0;
}
*/