#include "../include/uci.hpp"
#include "../include/board.hpp"
#include "../include/transposition.hpp"
#include <string>
#include <optional>
#include <iostream>

Board game_board;
std::optional<Transposition> game_table;

int main() {
    std::string line;
    bool quit = false;
    while (!quit) {
        std::getline(std::cin, line);
        quit = handle_command(line);
    }

    clean();

    return 0;
}