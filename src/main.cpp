#include <iostream>
#include <string>

#include "core/Engine.h"

int main()
{
    Engine engine;

    std::string command;

    std::cout << "=== Mini Backend Platform ===" << std::endl;
    std::cout << "Commands: SET key value | GET key | DELETE key | EXPIRE key seconds | RATE user | STATS | exit" << std::endl;

    while (true)
    {
        std::cout << "backend> " << std::flush;
        if (!std::getline(std::cin, command))
        {
            break;
        }

        if (command == "exit")
        {
            std::cout << "Shutting down..." << std::endl;
            break;
        }

        engine.process(command);
    }

    return 0;
}
