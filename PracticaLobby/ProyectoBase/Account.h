#pragma once
#include <string>
class Account
{
public:
	int id;
	std::string name;
	std::string paswd;
	int level;

	Account();
	~Account();
};

