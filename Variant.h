#ifndef VARIANT_H
#define VARIANT_H

#include <string>
#include <list>
#include <map>

class Variant;

typedef std::list<Variant> VariantList;
typedef std::map<std::string, Variant> VariantMap;

/**
 * @brief Anytype concept implementation.
 * The Variant class is a holder of any-type Lua value.
 */
class Variant
{
public:

    enum Type {
        Type_Invalid = 0,
		Type_Null	 = 1,
        Type_Boolean = 2,
        Type_Integer = 3,
        Type_Real    = 4,
        Type_String  = 5,
        Type_List    = 6,
        Type_Map     = 7,

        MaxTypes = Type_Map + 1
    };

    Variant();
    Variant(Type type);
    Variant(const Variant &variant);
    Variant(bool value);
    Variant(int value);
    Variant(double value);
    Variant(const char *pValue);
    Variant(const std::string &value);
    Variant(const VariantList &value);
    Variant(const VariantMap &value);
    Variant& operator =(const Variant &variant);
    Variant& operator =(bool value);
    Variant& operator =(int value);
    Variant& operator =(double value);
    Variant& operator =(const char *pValue);
    Variant& operator =(const std::string &value);
    Variant& operator =(const VariantList &value);
    Variant& operator =(const VariantMap &value);
    ~Variant();

    Type type() const { return m_type; }
    bool isValid() const { return m_type != Type_Invalid; }
    bool isNull() const { return m_type == Type_Null; }
    void clear();

    bool toBoolean(bool def = false) const;
    int toInteger(int def = 0) const;
    double toReal(double def = 0.0) const;
    std::string toString(const std::string &def = "") const;

    std::string& string();
    const std::string& string() const;
    VariantList& list();
    const VariantList& list() const;
    VariantMap& map();
    const VariantMap& map() const;

    friend std::ostream& operator <<(std::ostream &output, const Variant &variant);

private:

    void initializeType();
    void initFrom(const Variant &variant);

    Type m_type;    ///< Value type.

    union Data {
        bool b;		///< Boolean value.
        int i;		///< Integer value.
        double r;	///< Real value.
        void *ptr;	///< Pointer to object-based value (string, list, map)
    } m_data;
};

#endif // VARIANT_H
