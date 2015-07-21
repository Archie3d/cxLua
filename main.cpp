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

// Function to be exposed to the Lua engine
Variant lua_test(const VariantList &args, void *pData)
{
    (void)pData;    // Not used

    // Print passed arguments
    for (VariantList::const_iterator it = args.begin(); it != args.end(); ++it) {
        std::cout << (*it) << std::endl;
    }

    return Variant();
}

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

    // 5. Expose C++ object to Lua
    MyUnit myUnit;
    lua.registerObject("MyUnit", &myUnit);
    lua.evaluate("local s = MyUnit.sum(1, 2, 3, 4, 5)\n"
                 "print('sum = ' .. s)");

    // 6. Expose function to Lua
    lua.registerFunction("test", lua_test);
    lua.evaluate("test('a', 'b', 'c')");

    // 7. Expose lambda-function to Lua
    lua.registerFunction("test2", [](const VariantList &args, void *pData) -> Variant {
                            for (VariantList::const_iterator it = args.begin(); it != args.end(); ++it) {
                                std::cout << (*it) << std::endl;
                            }
                            return Variant();
                         });
    lua.evaluate("test2(1, 2, 3)");

    return 0;
}
