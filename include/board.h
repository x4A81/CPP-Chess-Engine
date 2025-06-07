#ifndef BOARD_H_INCLUDE
#define BOARD_H_INCLUDE

#include <cstdint>
#include <string>
#include <array>

using BB = uint64_t;

// Little-Endian Rank-File Mapping
enum Squares : int {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, no_square
};

/* Pieces indexes 
 
eg usage:
board.bitboards[b] // get the black bishop bitboard)

*/

enum Pieces : int {
    p, n, b, r, q, k, P, N, B, R, Q, K, bpieces = 12, wpieces, allpieces, no_piece
};

enum Colours : int {
    black, white, no_colour
};

/* Used for encoding castling rights:
eg:

int castling_rights = wking_side | wqueen_side;

*/

enum CastlingRights : int {
    wking_side = 1,
    wqueen_side,
    bking_side = 4,
    bqueen_side = 8
};

class Board {
public:
    Board() {
        reset();
    }

    Board(std::string fen) {
        reset();
        load_fen(fen);
    }

    Board(int load_start) {
        reset();
        if (!load_start) return;

        load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }

    std::array<BB, 15>bitboards{}; // p...kP...K black, white, all
    std::array<Pieces, 64>piece_list{};
    Squares enpassant_square;
    Colours side_to_move;
    uint8_t castling_rights;
    int halfmove_clock;
    int fullmove_counter;

    void reset();
    void load_fen(std::string fen);
    void print_board();
};

#endif