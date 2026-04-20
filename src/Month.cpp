#include "Month.h"


namespace
{
    const std::string MonthTable[12] =
    {
      "Jan",
      "Feb",
      "Mar",
      "Apr",
      "May",
      "Jun",
      "Jul",
      "Aug",
      "Sep",
      "Oct",
      "Nov",
      "Dec"
    };
}


const std::string& Month::toString(Month::Enum month)
{
    return MonthTable[static_cast<int>(month)];
}
