#include "ini.h"
#include <iostream>

int main(void)
{
    ini ini("test.ini");

    if (!ini.open())
        return -1;

    std::cout << ini.get("base info", "name", "none")<< std::endl;
    ini.erase("base info", "reserved");
    ini.set("base info", "age", 49);
    ini.save("test.backup.ini");
    ini.erase("base info");
    ini.save("test.backup1.ini");
    ini.clear();
    ini.save("test.backup2.ini");

    return 0;
}

