#pragma once



#include <string>

class Month
{
public:
	enum class Enum
	{
		Jan,
		Feb,
		Mar,
		Apr,
		May,
		Jun,
		Jul,
		Aug,
		Sep,
		Oct,
		Nov,
		Dec
	};

public:
	static const std::string& toString(Month::Enum month);
};
