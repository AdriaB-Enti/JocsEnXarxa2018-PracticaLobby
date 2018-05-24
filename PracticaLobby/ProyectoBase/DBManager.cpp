#include "DBManager.h"



DBManager::DBManager()
{
	driver = get_driver_instance();
	con = driver->connect(DB_HOST, USER, PWD);
	stmt = con->createStatement();
	stmt->execute("USE gamedb");
}

bool DBManager::registerUser(std::string name, std::string pswd)
{
	bool succes = false;

	//Comprovacions de si es pot registrar
	resultSet = stmt->executeQuery("select count(*) from Players where name="+name);
	if (resultSet->next())
	{
		if (resultSet->getInt(1) == 0) {
			stmt->execute("INSERT INTO Players(name, password) VALUES('"+name+"','"+pswd+"')");
			succes = true;
		}
	}
	//potser falten comprovacions

	return succes;
}

bool DBManager::login(std::string name, std::string pswd)
{



	return false;
}

void DBManager::addMatch(int idPlayer, int idSession)
{



}


DBManager::~DBManager()
{
}
