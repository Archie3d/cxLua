extern "C"
{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#include <sstream>
#include "LuaEngine.h"

/**
 * Global variable used to store script engine reference in
 * Lua VM context.
 */
const static char *cLuaScriptEngineRef = "__script_engine__";

/// Maximal allowed depth of Lua tables
const static int cLuaMaxTableLevel = 16;

static LuaEngine* getLuaEngine(lua_State *pLuaState)
{
	lua_getglobal(pLuaState, cLuaScriptEngineRef);
	void *ptr = lua_touserdata(pLuaState, -1);
	lua_pop(pLuaState, 1);

	return static_cast<LuaEngine*>(ptr);
}

static int scriptableObjectGateway(lua_State *pLuaState)
{
	LuaEngine *pLuaEngine = getLuaEngine(pLuaState);

	// Fetch method name
	std::string methodName(lua_tostring(pLuaState, lua_upvalueindex(1)));

	// Fetch object pointer
	void *ptr = lua_touserdata(pLuaState, lua_upvalueindex(2));
	Scriptable *pScriptable = static_cast<Scriptable*>(ptr);

	// Get arguments
	int nArgs = lua_gettop(pLuaState);
	VariantList args;
	for (int i = 0; i < nArgs; i++) {
		args.push_front(pLuaEngine->popValue());
	}

	// Invoke the method
	Variant ret = pScriptable->invokeMethod(methodName, args);
	if (ret.isValid()) {
        pLuaEngine->pushValue(ret);
		return 1;
	}

    // No return value from the method
	return 0;
}

static int nativeFunctionGateway(lua_State *pLuaState)
{
    LuaEngine *pLuaEngine = getLuaEngine(pLuaState);

    // Get native function pointer
    void *ptr = lua_touserdata(pLuaState, lua_upvalueindex(1));
    LuaEngine::NativeFunction func = reinterpret_cast<LuaEngine::NativeFunction>(reinterpret_cast<size_t>(ptr));

    // Get user data
    void *pData = lua_touserdata(pLuaState, lua_upvalueindex(2));

    // Get number of arguments
    int nArgs = lua_gettop(pLuaState);

    // Fetch arguments
    VariantList args;
    for (int i = 0; i < nArgs; i++) {
        args.push_front(pLuaEngine->popValue());
    }

    // Call native function
    Variant res = func(args, pData);

    if (res.isValid()) {
        pLuaEngine->pushValue(res);
        return 1;
    }

    // No return value from the function
    return 0;
}

struct LuaEngine::Private
{
    lua_State *pLuaState;       ///< Lua VM state.
    bool internalLuaState;		///< Whether the Lua state is created by this class.
    int error;                  ///< Error code.
    std::string errorText;      ///< Error message.
};


/*
 *	class LuaEngine::Reference
 */

LuaEngine::Reference::Reference()
	: m_identifier(),
	  m_pLuaEngine(0)
{
}

LuaEngine::Reference::Reference(const std::string &identifier, LuaEngine *pLuaEngine)
	: m_identifier(identifier),
	  m_pLuaEngine(pLuaEngine)
{
}

LuaEngine::Reference::Reference(const Reference &ref)
	: m_identifier(ref.m_identifier),
	  m_pLuaEngine(ref.m_pLuaEngine)
{
}

LuaEngine::Reference& LuaEngine::Reference::operator =(const LuaEngine::Reference &ref)
{
	if (this != &ref) {
		m_identifier = ref.m_identifier;
		m_pLuaEngine = ref.m_pLuaEngine;
	}
	return *this;
}

LuaEngine::Reference& LuaEngine::Reference::operator =(const Variant &value)
{
	if (m_pLuaEngine) {
		m_pLuaEngine->setGlobalValue(m_identifier, value);
	}
	return *this;
}

Variant LuaEngine::Reference::value() const
{
	if (m_pLuaEngine) {
		return m_pLuaEngine->globalValue(m_identifier);
	}
	return Variant();
}

Variant LuaEngine::Reference::operator ()(const VariantList &args)
{
	if (m_pLuaEngine) {
		return m_pLuaEngine->invoke(m_identifier, args);
	}
	return Variant();
}

Variant LuaEngine::Reference::operator ()(const Variant &a1)
{
	if (m_pLuaEngine) {
        VariantList args;
        args.push_back(a1);
		return m_pLuaEngine->invoke(m_identifier, args);
	}
	return Variant();
}

Variant LuaEngine::Reference::operator ()(const Variant &a1,
                                          const Variant &a2)
{
	if (m_pLuaEngine) {
        VariantList args;
        args.push_back(a1);
        args.push_back(a2);
		return m_pLuaEngine->invoke(m_identifier, args);
	}
	return Variant();
}

Variant LuaEngine::Reference::operator ()(const Variant &a1,
                                          const Variant &a2,
                                          const Variant &a3)
{
	if (m_pLuaEngine) {
        VariantList args;
        args.push_back(a1);
        args.push_back(a2);
        args.push_back(a3);
		return m_pLuaEngine->invoke(m_identifier, args);
	}
	return Variant();
}

Variant LuaEngine::Reference::operator ()(const Variant &a1,
                                          const Variant &a2,
                                          const Variant &a3,
                                          const Variant &a4)
{
	if (m_pLuaEngine) {
        VariantList args;
        args.push_back(a1);
        args.push_back(a2);
        args.push_back(a3);
        args.push_back(a4);
		return m_pLuaEngine->invoke(m_identifier, args);
	}
	return Variant();
}

Variant LuaEngine::Reference::operator ()(const Variant &a1,
                                          const Variant &a2,
                                          const Variant &a3,
                                          const Variant &a4,
                                          const Variant &a5)
{
	if (m_pLuaEngine) {
        VariantList args;
        args.push_back(a1);
        args.push_back(a2);
        args.push_back(a3);
        args.push_back(a4);
        args.push_back(a5);
		return m_pLuaEngine->invoke(m_identifier, args);
	}
	return Variant();
}


/*
 * 	class LuaEngine
 */

LuaEngine::LuaEngine()
{
    m = new LuaEngine::Private();
    initLuaState();
}

LuaEngine::LuaEngine(lua_State *pLuaState)
{
    m = new LuaEngine::Private();
    initLuaState(pLuaState);
}

LuaEngine::~LuaEngine()
{
	if (m->internalLuaState) {
		lua_close(m->pLuaState);
	}
    delete m;
}

void LuaEngine::clearError()
{
    m->error = 0;
    m->errorText = "";
}

int LuaEngine::error() const
{
    return m->error;
}

std::string LuaEngine::errorText() const
{
    return m->errorText;
}

void LuaEngine::reset()
{
	lua_close(m->pLuaState);
	initLuaState();
}

Variant LuaEngine::evaluate(const std::string &script)
{
    const char *cStr = script.c_str();
    int top = lua_gettop(m->pLuaState);
    int err = luaL_dostring(m->pLuaState, cStr);
    popError(err);

    return popReturnValues(top);
}

Variant LuaEngine::evaluateFile(const std::string &fileName)
{
	clearError();
	int top = lua_gettop(m->pLuaState);
	int err = luaL_dofile(m->pLuaState, fileName.c_str());
	popError(err);

	return popReturnValues(top);
}

Variant LuaEngine::invoke(const std::string &funcName,
                          const VariantList &args)
{
    int top = lua_gettop(m->pLuaState);

    lua_getglobal(m->pLuaState, funcName.c_str());
    for (VariantList::const_iterator it = args.begin(); it != args.end(); ++it) {
        pushValue(*it);
    }

    int err = lua_pcall(m->pLuaState, args.size(), LUA_MULTRET, 0);
    popError(err);

    return popReturnValues(top);
}

void LuaEngine::registerObject(const std::string &objectName, Scriptable *pScriptable)
{
	if (pScriptable == 0) {
		return;
	}

	lua_newtable(m->pLuaState);
	const Scriptable::MetaMethodsTable &methods = pScriptable->methods();

	for (Scriptable::MetaMethodsTable::const_iterator it = methods.begin(); it != methods.end(); ++it) {
		std::string name = it->first;
		pushString(name);
		pushString(name);
		pushData(static_cast<void*>(pScriptable));
		lua_pushcclosure(m->pLuaState, scriptableObjectGateway, 2);
		lua_settable(m->pLuaState, -3);
	}

	lua_setglobal(m->pLuaState, objectName.c_str());
}

void LuaEngine::registerFunction(const std::string &funcName, LuaEngine::NativeFunction func, void *pData)
{
    if (func) {
        pushData(reinterpret_cast<void*>(reinterpret_cast<size_t>(func)));
        pushData(pData);
        lua_pushcclosure(m->pLuaState, nativeFunctionGateway, 2);
        lua_setglobal(m->pLuaState, funcName.c_str());
    }
}

Variant LuaEngine::globalValue(const std::string &identifier)
{
	int top = lua_gettop(m->pLuaState);
	lua_getglobal(m->pLuaState, identifier.c_str());
	return popReturnValues(top);
}

void LuaEngine::setGlobalValue(const std::string &identifier, const Variant &value)
{
	pushValue(value);
	lua_setglobal(m->pLuaState, identifier.c_str());
}

LuaEngine::Reference LuaEngine::operator [](const std::string &identifier)
{
	return Reference(identifier, this);
}

void LuaEngine::pushValue(const Variant &value)
{
    switch (value.type()) {
    case Variant::Type_Boolean:
        pushBoolean(value.toBoolean());
        break;
    case Variant::Type_Integer:
        pushInteger(value.toInteger());
        break;
    case Variant::Type_Real:
        pushReal(value.toReal());
        break;
    case Variant::Type_String:
        pushString(value.toString());
        break;
    case Variant::Type_List: {
        const VariantList &list = value.list();
        lua_newtable(m->pLuaState);
        int i = 1; // Lua array index starts with 1
        for (VariantList::const_iterator it = list.begin(); it != list.end(); ++it, ++i) {
            pushInteger(i);
            pushValue(*it);
            lua_settable(m->pLuaState, -3);
        }
        break;
    }
    case Variant::Type_Map: {
        const VariantMap &map = value.map();
        lua_newtable(m->pLuaState);
        VariantMap::const_iterator it = map.begin();
        while (it != map.end()) {
            pushString(it->first);
            pushValue(it->second);
            lua_settable(m->pLuaState, -3);
            ++it;
        }
        break;
    }
    default:
    	// For invalid and null
        pushNull();
        break;
    }
}

Variant LuaEngine::popValue()
{
	return popValueSafe(cLuaMaxTableLevel);
}

Variant LuaEngine::popValueSafe(int tableLevel)
{
    Variant res;

    int luaType = lua_type(m->pLuaState, -1);
    switch (luaType) {
    case LUA_TNIL:
    	res = Variant(Variant::Type_Null);
    	break;
    case LUA_TBOOLEAN:
        res = toBoolean();
        break;
    case LUA_TNUMBER:
        res = toReal();
        break;
    case LUA_TSTRING:
        res = toString();
        break;
    case LUA_TTABLE: {

    	if (tableLevel == 0) {
    		lua_pop(m->pLuaState, 1);
    		return Variant();
    	}

        VariantMap map;
        VariantList list;
        pushNull();
        while (lua_next(m->pLuaState, -2)) {
            if (lua_type(m->pLuaState, -2) == LUA_TSTRING) {
                // Key is a string => constructing a map;
                const char *cKey = lua_tostring(m->pLuaState, -2);
                map[std::string(cKey)] = popValueSafe(tableLevel - 1);
            } else {
                list.push_back(popValueSafe(tableLevel - 1));
            }
        }

        if (map.empty()) {
            if (list.empty()) {
                res = map;
            } else {
                res = list;
            }
        } else {
            if (!list.empty()) {
                // Append list entries to the map
                int i = 1;
                for (VariantList::const_iterator it = list.begin(); it != list.end(); ++it, ++i) {
                    std::ostringstream ss;
                    ss << i;
                    std::string key = ss.str();
                    map[key] = *it;
                }
            }
            res = map;
        }
        break;
    }
    case LUA_TFUNCTION: {
        int ref = luaL_ref(m->pLuaState, LUA_REGISTRYINDEX);
        res = ref;
        return res; // Do not pop function reference
    }
    default:
        break;
    }

    lua_pop(m->pLuaState, 1);
    return res;
}

void LuaEngine::initLuaState(lua_State *pLuaState)
{
	if (pLuaState == 0) {
		m->pLuaState = luaL_newstate();
		m->internalLuaState = true;

		// Load Lua libraries
	    luaL_openlibs(m->pLuaState);
	} else {
		m->pLuaState = pLuaState;
		m->internalLuaState = false;
	}

    // Inject reference to this Lua engine object
    injectLuaEngineRef();

    clearError();
}


void LuaEngine::injectLuaEngineRef()
{
    void *ptr = static_cast<void*>(this);
    pushData(ptr);
    lua_setglobal(m->pLuaState, cLuaScriptEngineRef);
}

void LuaEngine::popError(int err)
{
    if (err != 0) {
        m->error = err;
        const char *strErr = lua_tostring(m->pLuaState, -1);
        if (strErr) {
            lua_pop(m->pLuaState, 1);
            m->errorText = std::string(strErr);
        }
    }
}

void LuaEngine::pushNull()
{
    lua_pushnil(m->pLuaState);
}

void LuaEngine::pushBoolean(bool value)
{
    lua_pushboolean(m->pLuaState, value ? 1 : 0);
}

void LuaEngine::pushInteger(int value)
{
    lua_pushinteger(m->pLuaState, value);
}

void LuaEngine::pushReal(double value)
{
    lua_pushnumber(m->pLuaState, value);
}

void LuaEngine::pushString(const std::string &value)
{
    const char *cStr = value.c_str();
    lua_pushstring(m->pLuaState, cStr);
}

void LuaEngine::pushData(void *ptr)
{
    lua_pushlightuserdata(m->pLuaState, ptr);
}

bool LuaEngine::toBoolean()
{
    return (lua_toboolean(m->pLuaState, -1) == 0) ? false : true;
}

int LuaEngine::toInteger()
{
    return static_cast<int>(lua_tointeger(m->pLuaState, -1));
}

double LuaEngine::toReal()
{
    return lua_tonumber(m->pLuaState, -1);
}

std::string LuaEngine::toString()
{
    return std::string(lua_tostring(m->pLuaState, -1));
}

void* LuaEngine::toData()
{
    return lua_touserdata(m->pLuaState, -1);
}

Variant LuaEngine::popReturnValues(int top)
{
    if (isError()) {
        return Variant();
    }

    VariantList returnValues;
    int nresults = lua_gettop(m->pLuaState) - top;
    for (int i = 0; i < nresults; i++) {
        returnValues.push_front(popValue());
    }

    if (returnValues.size() == 0) {
        return Variant();
    }

    if (returnValues.size() == 1) {
        return returnValues.front();
    }

    return Variant(returnValues);
}
