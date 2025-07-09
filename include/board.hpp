#ifndef BOARD_HPP_INCLUDE
#define BOARD_HPP_INCLUDE

#include "misc.hpp"
#include <cstdint>
#include <string>
#include <array>
#include <stack>
#include <vector>
#include <chrono>

using namespace std;

class Board;
extern Board game_board;

/// @brief Bitboard type
using BB = uint64_t;

// @brief encoded squares in 'Little-Endian Rank-File Mapping' format.
// See https://www.chessprogramming.org/Square_Mapping_Considerations#Little-Endian_File-Rank_Mapping

using Square = int;
enum SquaresEncoding : Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, no_square
};

/// @brief Piece representation, also used for indexing.
using Piece = int;
enum PiecesEncoding : Piece {
    p, n, b, r, q, k, P, N, B, R, Q, K, bpieces = 12, wpieces, allpieces, no_piece
};

/// @brief Colour representation, also used for indexing.
using Colour = int;
enum ColoursEncoding : Colour {
    black = 0, white = 1, no_colour = 2
};

/// @brief Colour helper.
/// @param friendly_colour Colour to get the opposite of.
/// @return The opposite colour to friendly_colour, no_colour if friendly_colour is no_colour.
inline Colour opposition_colour(Colour friendly_colour) {
    if (friendly_colour == no_colour) return no_colour;
    return friendly_colour ^ 1;
}

/// @brief Castling rights encoder.
using CastlingRights = int;
enum CastlingRightsEncoder : CastlingRights {
    wking_side = 1,
    wqueen_side,
    bking_side = 4,
    bqueen_side = 8
};

/// @brief Castling rights helper.
inline constexpr array<int, 64> castle_encoder = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7, 15, 15, 15,  3, 15, 15, 11
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

using Move = uint16_t;
#define nullmove 0

using Code = int;

enum MoveCode : Code {
    quiet, dbpush, kcastle, qcastle, capture, epcapture, 
    npromo = 8, bpromo, rpromo, qpromo, c_npromo, c_bpromo, c_rpromo, c_qpromo
};

/// @brief Move helper.
/// @param move Move to compare.
/// @param code Code to compare.
/// @return If the move is of type code.
inline bool is_move(Move move, MoveCode code)  { return (move >> 12) == code; }

/// @brief Move helper
/// @param move Encoded move.
/// @return The from square of the move.
inline Square get_from_sq(Move move) { return move & 0b111111; }

/// @brief Move helper
/// @param move Encoded move.
/// @return The to square of the move.
inline Square get_to_sq(Move move) { return (move >> 6) & 0b111111; }

/// @brief Move helper
/// @param move Encoded move.
/// @return The code of the move.
inline Square get_code(Move move) { return move >> 12; }

constexpr int MAX_MOVE_LIST_SIZE = 256;

class MoveList {
private:
    array<Move, MAX_MOVE_LIST_SIZE> _moves;
    size_t _size;

public:
    void add(Move move) { _moves.at(_size++) = move; }

    void clear() {
        _moves.fill(0);
        _size = 0;
    }
    
    size_t size() { return _size; }
    auto begin() { return _moves.begin(); };
    auto end() { return _moves.begin() + _size; }
    bool is_empty() { return _size == 0 ? true : false; }
    const Move& operator[](size_t idx) const {
        return _moves[idx];
    }

    Move& operator[](size_t idx) {
        return _moves[idx];
    }
};

using KEY = uint64_t;
struct Board_State {
    array<BB, 15>bitboards{}; // p...kP...K black, white, all
    array<Piece, 64>piece_list{};
    Square enpassant_square;
    Colour side_to_move;
    uint8_t castling_rights;
    int halfmove_clock;
    int fullmove_counter;
    KEY hash_key;
    MoveList move_list;
    void reset();
};

class Board;

using KEY = uint64_t;
namespace zobrist {
    inline array<KEY, 768> piece_keys; // 12 * 64
    inline array<KEY, 4> castling_keys;
    inline array<KEY, 8> ep_file_key;
    inline KEY side_key;

    void init_keys();
    KEY gen_pos_key(Board_State& state);
}

#define MAX_PLY 64
#define MAX_DEPTH 20
#define PV_TABLE_SIZE (MAX_PLY*MAX_PLY+MAX_PLY)/2

struct SearchState {
    int depth = 0;
    int ply = 0;
    long nodes = 0;
    chrono::steady_clock::time_point start_time;
    array<Move, PV_TABLE_SIZE> pv_table = { nullmove };
    array<int, MAX_PLY> pv_length = { 0 };
    array<array<Move, 2>, MAX_PLY> killer_moves = {{ nullmove }};
    array<array<array<int, 2>, 64>, 64> history_moves {};
    Move fallback_move = nullmove;
};

struct SearchParams;

#define CAPTURES true
#define ALLMOVES false

class Board {
private:
    vector<Board_State> prev_states;
    Move generate_move_nopromo(Square from_sq, Square to_sq);
    int quiescence(int alpha, int beta);
    int search(int depth, int alpha, int beta);
    bool is_search_stopped();
    void order_moves(Move hash_move);

public:
    SearchState search_state;
    Board_State state;
    bool is_in_check;

    Board() {
        move_generator::init_sliding_move_tables();
        zobrist::init_keys();
        state.reset();
        prev_states.push_back(state);
    }

    Board(string fen) {
        move_generator::init_sliding_move_tables();
        zobrist::init_keys();
        load_fen(fen);
        prev_states.push_back(state);
    }

    Board(int load_start) {
        if (!load_start) return;
        move_generator::init_sliding_move_tables();
        load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        prev_states.push_back(state);
        zobrist::init_keys();
    }

    void load_fen(string fen);
    void print_board();

    template <bool GEN_CAPTURES>
    void generate_moves();
    void make_move(Move move);
    void unmake_last_move();
    int eval();
    void run_search(SearchParams params);
    bool is_draw();
    bool is_over();
    Colour winner();
};

#endif