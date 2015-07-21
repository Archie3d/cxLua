#include "Scriptable.h"

Scriptable::Scriptable()
	: m_metaMethods()
{
}

void Scriptable::registerMethod(const std::string &methodName, Method method)
{
	m_metaMethods[methodName] = method;
}

Variant Scriptable::invokeMethod(const std::string &methodName,
								 const VariantList &args)
{
	Variant res;

	MetaMethodsTable::const_iterator it = m_metaMethods.find(methodName);
	if (it == m_metaMethods.end()) {
		// Method not found!
		return Variant();
	}

	Method method = it->second;

	return (*this.*method)(args);
}


