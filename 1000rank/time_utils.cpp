#include "time_utils.h"
#include <iomanip>
#include <sstream>

int date_string_to_unix_time(const std::string& date_str) {
    std::tm tm = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    return static_cast<int>(mktime(&tm));
}