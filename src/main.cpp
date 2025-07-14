#include "../include/uci.hpp"
#include "../include/board.hpp"
#include "../include/transposition.hpp"
#include "../include/book.hpp"
#include <string>
#include <optional>
#include <iostream>
#include <filesystem>

Board game_board;
std::optional<Transposition> game_table;
std::filesystem::path book_path;

int main(int argc, char* argv[]) {
    std::filesystem::path exe_path = argv[0];
    std::filesystem::path exe_dir = exe_path.parent_path();
    book_path = exe_dir / "book.bin";

    std::string line;
    bool quit = false;
    while (!quit) {
        std::getline(std::cin, line);
        quit = handle_command(line);
    }

    clean();

    return 0;
}