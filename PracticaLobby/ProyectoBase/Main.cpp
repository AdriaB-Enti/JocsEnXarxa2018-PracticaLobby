//Proves DML-SQL
#pragma once

#include <stdlib.h>
#include <iostream>
#include <mysql_connection.h>
#include <cppconn\driver.h>
#include <cppconn\resultset.h>
#include <cppconn\statement.h>
#include <cppconn\exception.h>

#include "DBManager.h"

//tenir un unic punt d'entrada -recieve

int main()
{
	//Testing user register
	DBManager dbm = DBManager();
	dbm.registerUser("testingName", "passwordTest");


	system("pause");
	return 0;
}