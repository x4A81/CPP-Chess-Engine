#ifndef BOARD_H_INCLUDE
#define BOARD_H_INCLUDE

#include <cstdint>
#include <string>
#include <array>
#include <stack>
#include <vector>

/// @brief Bitboard type
using BB = uint64_t;

// @brief encoded squares in 'Little-Endian Rank-File Mapping' format.
// See https://www.chessprogramming.org/Square_Mapping_Considerations#Little-Endian_File-Rank_Mapping
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

enum Castling_Rights : int {
    wking_side = 1,
    wqueen_side,
    bking_side = 4,
    bqueen_side = 8
};

/// Move encoding:
/// 6 bits for the from square
/// 6 bits for the to square
// 4 bits for a code
// code  | Promo | capt | special 1 | sp 2 | type
/// 0    | 0     | 0    | 0         | 0    | quiet
/// 1    | 0     | 0    | 0         | 1    | dbp pawn
/// 2    | 0     | 0    | 1         | 0    | king castle
/// 3    | 0     | 0    | 1         | 1    | q castle
/// 4    | 0     | 1    | 0         | 0    | capture
/// 5    | 0     | 1    | 0         | 1    | ep capture
/// 8    | 1     | 0    | 0         | 0    | knight promo
/// 9    | 1     | 0    | 0         | 1    | bishop promo
/// 10   | 1     | 0    | 1         | 0    | rook promo
/// 11   | 1     | 0    | 1         | 1    | queen promo
/// 12   | 1     | 1    | 0         | 0    | knight promo capt
/// 13   | 1     | 1    | 0         | 1    | bishop promo capt
/// 14   | 1     | 1    | 1         | 0    | rook promo capt
/// 15   | 1     | 1    | 1         | 1    | queen promo capt
using Move = int16_t;
#define nullmove 0

enum Move_Code : int {
    quiet, dbpush, kcastle, qcastle, capture, epcapture, 
    npromo = 8, bpromo, rpromo, qpromo, c_npromo, c_bpromo, c_rpromo, c_qpromo
};

class Move_List {
private:
    std::array<Move, 256> _moves;
    std::size_t _size;

public:
    void add(Move move) { _moves.at(_size++) = move; }

    void clear() {
        _moves.fill(0);
        _size = 0;
    }

    auto begin() { return _moves.begin(); };
    auto end() { return _moves.begin() + _size; }
    bool is_empty() { return _size == 0 ? true : false; }
};

using KEY = uint64_t;
struct Board_State {
    std::array<BB, 15>bitboards{}; // p...kP...K black, white, all
    std::array<Pieces, 64>piece_list{};
    Squares enpassant_square;
    Colours side_to_move;
    uint8_t castling_rights;
    int halfmove_clock;
    int fullmove_counter;
    KEY hash_key;
    Move_List move_list;
    void reset();
};

#define CAPTURES true
#define ALLMOVES false

class Board;

using KEY = uint64_t;
namespace zobrist {
    inline std::array<KEY, 768> piece_keys; // 12 * 64
    inline std::array<KEY, 4> castling_keys;
    inline std::array<KEY, 8> ep_file_key;
    inline KEY side_key;

    void init_keys();
    KEY gen_pos_key(Board_State& state);
}

class Board {
private:
    std::vector<Board_State> prev_states;
    Move generate_move_nopromo(Squares from_sq, Squares to_sq);
    
public:
    std::array<BB, 16> generate_move_targets();

    Board_State state;

    Board() {
        zobrist::init_keys();
        state.reset();
        prev_states.push_back(state);
    }

    Board(std::string fen) {
        zobrist::init_keys();
        load_fen(fen);
        prev_states.push_back(state);
    }

    Board(int load_start) {
        if (!load_start) return;

        load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        prev_states.push_back(state);
        zobrist::init_keys();
    }

    void load_fen(std::string fen);
    void print_board();

    template <bool GEN_CAPTURES>
    void generate_moves();
    void make_move(Move move);
    void unmake_last_move();

    bool is_draw();
    bool is_over();
    Colours winner();
};

#endif