#ifndef LUAENGINE_H
#define LUAENGINE_H

#include "Variant.h"
#include "Scriptable.h"

struct lua_State;

/**
 * C++ wrapper for Lua VM
 */
class LuaEngine
{
public:

    /// Lua global reference helper
	class Reference
	{
	public:
		Reference();
		Reference(const std::string &identifier, LuaEngine *pLuaEngine);
		Reference(const Reference &ref);
		Reference& operator =(const Reference &ref);
		Reference& operator =(const Variant &value);
		Variant value() const;
		Variant operator ()(const VariantList &args = VariantList());
		Variant operator ()(const Variant &a1);
		Variant operator ()(const Variant &a1, const Variant &a2);
		Variant operator ()(const Variant &a1, const Variant &a2, const Variant &a3);
		Variant operator ()(const Variant &a1, const Variant &a2, const Variant &a3, const Variant &a4);
		Variant operator ()(const Variant &a1, const Variant &a2, const Variant &a3, const Variant &a4, const Variant &a5);
	private:
		std::string m_identifier;
		mutable LuaEngine *m_pLuaEngine;
	};

    /// Native function
    typedef Variant (*NativeFunction)(const VariantList &args, void *pData);

    LuaEngine();
    LuaEngine(lua_State *pLuaState);
    ~LuaEngine();

    void clearError();
    int error() const;
    std::string errorText() const;
    bool isError() const { return error() != 0; }

    // Reset Lua environment.
    void reset();

    /**
     * Evaluate Lua script.
     */
    Variant evaluate(const std::string &script);

    Variant evaluateFile(const std::string &fileName);

    Variant invoke(const std::string &funcName,
                   const VariantList &args = VariantList());

    void registerObject(const std::string &objectName, Scriptable *pScriptable);

    void registerFunction(const std::string &funcName, NativeFunction func, void *pData = 0);

    Variant globalValue(const std::string &identifier);
    void setGlobalValue(const std::string &identifier, const Variant &value);

    Reference operator[](const std::string &identifier);

    void pushValue(const Variant &value);
    Variant popValue();

private:

    void initLuaState(lua_State *pLuaState = 0);
    void injectLuaEngineRef();
    void popError(int err);
    Variant popValueSafe(int tableLevel);

    void pushNull();
    void pushBoolean(bool value);
    void pushInteger(int value);
    void pushReal(double value);
    void pushString(const std::string &value);
    void pushData(void *pData);

    // Peek top-most value of corresponding data type
    bool toBoolean();
    int toInteger();
    double toReal();
    std::string toString();
    void* toData();

    Variant popReturnValues(int top);

    /**
     * Forward declaration of Lua engine data structures
     * to avoid including Lua headers in dependent files.
     */
    struct Private;
    Private *m;
};

#endif // LUAENGINE_H
