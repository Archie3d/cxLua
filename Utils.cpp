#include "Utils.h"

const char *cWhiteSpaceCharacters = " \t\n\r\f\v";

std::string& rtrim(std::string& s, const char* t)
{
    const char *wsc = t ? t : cWhiteSpaceCharacters;
    s.erase(s.find_last_not_of(wsc) + 1);
    return s;
}

std::string& ltrim(std::string& s, const char* t)
{
    const char *wsc = t ? t : cWhiteSpaceCharacters;
    s.erase(0, s.find_first_not_of(wsc));
    return s;
}

std::string& trim(std::string& s, const char* t)
{
    const char *wsc = t ? t : cWhiteSpaceCharacters;
    return ltrim(rtrim(s, wsc), wsc);
}
