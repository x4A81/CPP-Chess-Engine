#include "../include/uci.hpp"
#include <string>
#include <iostream>

int main() {
    std::string line;
    bool hasnt_recv_quit = true;
    while (hasnt_recv_quit) {
        std::getline(std::cin, line);
        hasnt_recv_quit = uci::handle_command(line);
    }

    uci::clean();
    return 0;
}