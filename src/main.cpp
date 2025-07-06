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
    bool hasnt_recv_quit = true;
    while (hasnt_recv_quit) {
        std::getline(std::cin, line);
        hasnt_recv_quit = handle_command(line);
    }

    clean();

    return 0;
}