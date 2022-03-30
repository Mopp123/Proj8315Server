#pragma once

#include <string>



class Faction
{
private:

	std::string _name;

public:

	Faction(const std::string& name) : 
		_name(name)
	{}
	
	Faction(const Faction& other) : 
		_name(other._name)
	{}

	inline const std::string& getName() const { return _name; }
};
