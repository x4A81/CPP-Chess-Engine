#include "../include/board.hpp"
#include <array>

constexpr std::array<int, 12> material = { 100, 320, 330, 500, 900, 1000, -100, -320, -330, -500, -900, -1000 };

int Board::eval() {
    int score = 0;

    for (Pieces piece : state.piece_list) {
        if (piece >= no_piece) continue;
        score += material[piece];
    }

    return (state.side_to_move == white) ? score : -score;
}