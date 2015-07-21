#ifndef SCRIPTABLE_H
#define SCRIPTABLE_H

#include <string>
#include <map>
#include "Variant.h"

/**
 * @brief Abstract scriptable class
 *
 * This is an abstract implementation of a class that
 * can be exposed to the Lua engine.
 */
class Scriptable
{
public:

    /**
     * Scriptable class method.
     */
	typedef Variant (Scriptable::*Method)(const VariantList &args);

	/**
	 * Table of scriptable methods.
	 */
	typedef std::map<std::string, Scriptable::Method> MetaMethodsTable;

	Scriptable();
	virtual ~Scriptable() {};

    /**
     * Register method to be exposed to the Lua engine.
     * @param methodName Method name as it will be seen in Lua environment.
     * @param method Corresponding native method.
     */
	void registerMethod(const std::string &methodName, Method method);

    /**
     * Invoke registered scriptable method.
     * This will be normally called by the Lua engine.
     * @param methodName Registered method name.
     * @param args List of arguments.
     * @return Method return value.
     */
	Variant invokeMethod(const std::string &methodName,
						 const VariantList &args = VariantList());

    /**
     * Number of registered scribtable methods.
     * @return Number of scriptable methods.
     */
	int methodCount() const { return m_metaMethods.size(); }

    /**
     * Returns reference to the table of scriptable methods.
     * @return Reference to the table of scriptable methods.
     */
	const MetaMethodsTable& methods() const { return m_metaMethods; }

private:

	MetaMethodsTable m_metaMethods; ///< Table of registered scriptable methods.
};


#endif // SCRIPTABLE_H
