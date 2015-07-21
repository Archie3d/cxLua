#include <iostream>
#include "LuaEngine.h"

//
// Example of Lua Engine usage
//

// Class that will be exposed to the Lua engine
class MyUnit : public Scriptable
{
public:
    MyUnit()
    {
        // Register all methods to be exposed to Lua engine
        registerMethod("sum", static_cast<Method>(&MyUnit::sum));
    }

    //------------------------------------------------------
    // Scriptable methods
    //------------------------------------------------------

    // Sum all arguments
    Variant sum(const VariantList &args)
    {
        double res = 0.0;
        for (VariantList::const_iterator it = args.begin(); it != args.end(); ++it) {
            res += (*it).toReal();
        }
        return res;
    }

};

int main()
{
    LuaEngine lua;

    // 1. Evaluate Lua script
    lua.evaluate("function add(x, y) return x + y end");
    if (lua.isError()) {
        std::cerr << "Lua error: " << lua.errorText() << std::endl;
    }

    // 2. Invoke Lua function
    Variant res = lua["add"](2, 3);
    std::cout << "res = " << res << std::endl;

    // 3. Assign Lua global variable
    lua["GlobalValue"] = "test";

    // 4. Read Lua global variable
    std::cout << "GlobalValue = " << lua["GlobalValue"].value() << std::endl;

    // 5. Expose native code to Lua
    MyUnit myUnit;
    lua.registerObject("MyUnit", &myUnit);
    lua.evaluate("local s = MyUnit.sum(1, 2, 3, 4, 5)\n"
                 "print('sum = ' .. s)");

    return 0;
}
