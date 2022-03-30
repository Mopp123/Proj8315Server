#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

class Debug
{
public:

	static void log(const std::string& msg)
	{
		std::cout << msg << std::endl;
	}
};
