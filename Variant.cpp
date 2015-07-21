#include <string.h>
#include <sstream>
#include <iomanip>
#include "Utils.h"
#include "Variant.h"


Variant::Variant()
    : m_type(Type_Invalid)
{
    m_data.ptr = 0;
}

Variant::Variant(Type type)
    : m_type(type)
{
    m_data.ptr = 0;
    initializeType();
}

Variant::Variant(const Variant &variant)
    : m_type(variant.m_type)
{
    initFrom(variant);
}

Variant::Variant(bool value)
    : m_type(Type_Boolean)
{
    m_data.b = value;
}

Variant::Variant(int value)
    : m_type(Type_Integer)
{
    m_data.i = value;
}

Variant::Variant(double value)
    : m_type(Type_Real)
{
    m_data.r = value;
}

Variant::Variant(const char *pValue)
    : m_type(Type_String)
{
    m_data.ptr = new std::string(pValue);
}

Variant::Variant(const std::string &value)
    : m_type(Type_String)
{
    m_data.ptr = new std::string(value);
}

Variant::Variant(const VariantList &value)
    : m_type(Type_List)
{
    m_data.ptr = new VariantList(value);
}

Variant::Variant(const VariantMap &value)
    : m_type(Type_Map)
{
    m_data.ptr = new VariantMap(value);
}

Variant& Variant::operator =(const Variant &variant)
{
    if (this != &variant) {
        clear();
        m_type = variant.m_type;
        initFrom(variant);
    }
    return *this;
}

Variant& Variant::operator =(bool value)
{
    if (m_type != Type_Boolean) {
        clear();
        m_type = Type_Boolean;
    }
    m_data.b = value;
    return *this;
}

Variant& Variant::operator =(int value)
{
    if (m_type != Type_Integer) {
        clear();
        m_type = Type_Integer;
    }
    m_data.i = value;
    return *this;
}

Variant& Variant::operator =(double value)
{
    if (m_type != Type_Real) {
        clear();
        m_type = Type_Real;
    }
    m_data.r = value;
    return *this;
}

Variant& Variant::operator =(const char *pValue)
{
    return operator =(std::string(pValue));
}

Variant& Variant::operator =(const std::string &value)
{
    if (m_type != Type_String) {
        clear();
        m_type = Type_String;
        m_data.ptr = new std::string(value);
    } else {
        std::string *pString = static_cast<std::string*>(m_data.ptr);
        *pString = value;
    }
    return *this;
}

Variant& Variant::operator =(const VariantList &value)
{
    if (m_type != Type_List) {
        clear();
        m_type = Type_List;
        m_data.ptr = new VariantList(value);
    } else {
        VariantList *pList = static_cast<VariantList*>(m_data.ptr);
        *pList = value;
    }
    return *this;
}

Variant& Variant::operator =(const VariantMap &value)
{
    if (m_type != Type_Map) {
        clear();
        m_type = Type_Map;
        m_data.ptr = new VariantMap(value);
    } else {
        VariantMap *pMap = static_cast<VariantMap*>(m_data.ptr);
        *pMap = value;
    }
    return *this;
}

Variant::~Variant()
{
    clear();
}

void Variant::clear()
{
    switch (m_type) {
    case Type_String: {
        std::string *pStr = static_cast<std::string*>(m_data.ptr);
        delete pStr;
        break;
    }
    case Type_List: {
        VariantList *pList = static_cast<VariantList*>(m_data.ptr);
        delete pList;
        break;
    }
    case Type_Map: {
        VariantMap *pMap = static_cast<VariantMap*>(m_data.ptr);
        delete pMap;
        break;
    }
    default:
        break;
    }

    memset(&m_data, 0, sizeof(Data));
    m_type = Type_Invalid;
}

bool Variant::toBoolean(bool def) const
{
    bool res = def;
    switch (m_type) {
    case Type_Boolean:
        res = m_data.b;
        break;
    case Type_Integer:
        res = m_data.i != 0;
        break;
    case Type_String: {
        std::string *pStr = static_cast<std::string*>(m_data.ptr);
        res = (*pStr == "true");
        break;
    }
    default:
        break;
    }

    return res;
}

int Variant::toInteger(int def) const
{
    int res = def;
    switch (m_type) {
    case Type_Boolean:
        res = m_data.b ? 1 : 0;
        break;
    case Type_Integer:
        res = m_data.i;
        break;
    case Type_Real:
        res = static_cast<int>(m_data.r);
        break;
    case Type_String: {
        std::string *pStr = static_cast<std::string*>(m_data.ptr);
        res = stringToNumber<int>(*pStr);
        break;
    }
    default:
        break;
    }

    return res;
}

double Variant::toReal(double def) const
{
    double res = def;
    switch (m_type) {
    case Type_Boolean:
        res = m_data.b ? 1.0 : 0.0;
        break;
    case Type_Integer:
        res = static_cast<double>(m_data.i);
        break;
    case Type_Real:
        res = m_data.r;
        break;
    case Type_String: {
        std::string *pStr = static_cast<std::string*>(m_data.ptr);
        res = stringToNumber<double>(*pStr);
        break;
    }
    default:
        break;
    }

    return res;
}

std::string Variant::toString(const std::string &def) const
{
    std::string res(def);

    switch (m_type) {
    case Type_Invalid:
    	res = "invalid";
    	break;
    case Type_Null:
    	res = "null";
    	break;
    case Type_Boolean:
        res = m_data.b ? "true" : "false";
        break;
    case Type_Integer:
        res = numberToString(m_data.i);
        break;
    case Type_Real:
        res = numberToString(m_data.r);
        break;
    case Type_String: {
        std::string *pStr = static_cast<std::string*>(m_data.ptr);
        res = *pStr;
        break;
    }
    case Type_List: {
        VariantList *pList = static_cast<VariantList*>(m_data.ptr);
        res = "[";
        bool first = true;
        for (VariantList::const_iterator it = pList->begin(); it != pList->end(); ++it) {
            if (!first) {
                res.append(", ");
            } else {
                first = false;
            }
            res.append((*it).toString());
        }
        res.append("]");
        break;
    }
    case Type_Map: {
        VariantMap *pMap = static_cast<VariantMap*>(m_data.ptr);
        res = "{";
        VariantMap::const_iterator i = pMap->begin();
        while (i != pMap->end()) {
            res.append(i->first);
            res.append(": ");
            res.append(i->second.toString());
            ++i;
            if (i != pMap->end()) {
                res.append(", ");
            }
        }
        res.append("}");
        break;
    }
    default:
        break;
    }

    return res;
}

std::string& Variant::string()
{
    return *static_cast<std::string*>(m_data.ptr);
}

const std::string& Variant::string() const
{
    return *static_cast<std::string*>(m_data.ptr);
}

VariantList& Variant::list()
{
    return *static_cast<VariantList*>(m_data.ptr);
}

const VariantList& Variant::list() const
{
    return *static_cast<VariantList*>(m_data.ptr);
}

VariantMap& Variant::map()
{
    return *static_cast<VariantMap*>(m_data.ptr);
}

const VariantMap& Variant::map() const
{
    return *static_cast<VariantMap*>(m_data.ptr);
}

std::ostream& operator <<(std::ostream &output, const Variant &variant)
{
    return output << variant.toString();
}

void Variant::initializeType()
{
    switch (m_type) {
    case Type_String:
        m_data.ptr = new std::string();
        break;
    case Type_List:
        m_data.ptr = new VariantList();
        break;
    case Type_Map:
        m_data.ptr = new VariantMap();
        break;
    default:
        break;
    }
}

void Variant::initFrom(const Variant &variant)
{
    switch (m_type) {
    case Type_String: {
        std::string *pStr = static_cast<std::string*>(variant.m_data.ptr);
        m_data.ptr = new std::string(*pStr);
        break;
    }
    case Type_List: {
        VariantList *pList = static_cast<VariantList*>(variant.m_data.ptr);
        m_data.ptr = new VariantList(*pList);
        break;
    }
    case Type_Map: {
        VariantMap *pMap = static_cast<VariantMap*>(variant.m_data.ptr);
        m_data.ptr = new VariantMap(*pMap);
        break;
    }
    default:
        memcpy(&m_data, &variant.m_data, sizeof(Data));
        break;
    }
}
