#include <chrono>
#include <iostream>
#include <string>
#include <thread>

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " <service-port> <db-host> <db-port>" << std::endl;
}

int main(int argc, char const *argv[])
{
    auto iter = 0;
    while (true)
    {
        std::cout << "this is iter " << iter++ << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}
