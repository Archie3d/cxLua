#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <string>
#include <sstream>

std::string& rtrim(std::string &s, const char *t = 0);
std::string& ltrim(std::string &s, const char *t = 0);
std::string& trim(std::string &s, const char *t = 0);

template <typename T>
std::string numberToString(T number)
{
    std::ostringstream ss;
    ss << number;
    return ss.str();
}

template <typename T>
T stringToNumber(const std::string &s)
{
    std::istringstream ss(s);
    T res;
    ss >> res;
    return res;
}

#endif // UTILS_H_INCLUDED
