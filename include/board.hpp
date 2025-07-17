#ifndef BOARD_HPP_INCLUDE
#define BOARD_HPP_INCLUDE

#include <cstdint>
#include <string>
#include <array>
#include <vector>

#include "move_gen.hpp"
#include "globals.hpp"

class Board;
extern Board game_board;

/// @brief Castling rights encoder.
using CastlingRights = int;
enum CastlingRightsEncoder : CastlingRights {
    wking_side = 1,
    wqueen_side,
    bking_side = 4,
    bqueen_side = 8
};

/// @brief Castling rights helper.
inline constexpr std::array<CastlingRights, 64> castle_encoder = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7, 15, 15, 15,  3, 15, 15, 11
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
inline Code get_code(Move move) { return move >> 12; }

/// @brief Move helper. Similar to is_move(Move, MoveCode) but includes promo captures.
/// @param move Move to compare.
/// @return true if move is capture.
inline bool is_move_capture(Move move) { 
    return is_move(move, capture) || get_code(move) >= c_npromo;
}

/// @brief Square helper.
/// @param sq The square to flip.
/// @return The reflected square across the rank axis.
inline Square flip_rank(Square sq) { return sq ^ 56; }

inline int get_file(Square sq) { return sq & 7; }
inline int get_rank(Square sq) { return sq >> 3; }

/// @brief Colour helper.
/// @param friendly_colour Colour to get the opposite of.
/// @return The opposite colour to friendly_colour, no_colour if friendly_colour is no_colour.
inline Colour opposition_colour(Colour friendly_colour) {
    if (friendly_colour == no_colour) return no_colour;
    return friendly_colour ^ 1;
}

constexpr int MAX_MOVE_LIST_SIZE = 256;

class MoveList {
private:
    std::array<Move, MAX_MOVE_LIST_SIZE> _moves;
    size_t _size;

public:
    [[gnu::hot]]
    void add(Move move) { _moves.at(_size++) = move; }

    void clear() {
        _moves.fill(nullmove);
        _size = 0;
    }
    
    size_t size() { return _size; }
    auto begin() { return _moves.begin(); };
    auto end() { return _moves.begin() + _size; }
    bool is_empty() { return _size == 0; }
    const Move& operator[](size_t idx) const {
        return _moves[idx];
    }

    Move& operator[](size_t idx) {
        return _moves[idx];
    }
};

struct BoardState {
    std::array<BB, 15>bitboards{}; // p...kP...K black, white, all
    std::array<Piece, 64>piece_list{};
    Square enpassant_square;
    Colour side_to_move;
    uint8_t castling_rights;
    int halfmove_clock;
    int fullmove_counter;
    Key hash_key;
    MoveList move_list;
    bool is_in_check = 0;
    void reset();
};

namespace zobrist {
    inline std::array<Key, 768> piece_keys; // 12 * 64
    inline std::array<Key, 4> castling_keys;
    inline std::array<Key, 8> ep_file_key;
    inline Key side_key;

    void init_keys();
    Key gen_pos_key(BoardState& state);
}

#define UNUSED -1

struct SearchParams {
    int max_depth = UNUSED;
    int nodes = UNUSED;
    int move_time = UNUSED;
    int wtime = UNUSED;
    int btime = UNUSED;
    int winc = UNUSED;
    int binc = UNUSED;
    int movestogo = UNUSED;
    bool infinite = false;

    SearchParams(const SearchParams&) = default;
    SearchParams() = default;

    // Unsupported
    // bool ponder = false;
};

#define CAPTURES true
#define ALLMOVES false
#define PV_TABLE_SIZE (max_ply*max_ply+max_ply)/2

class Board {
private:
    std::vector<BoardState> prev_states;
    std::array<int, max_ply> pv_length = { 0 };
    void update_pv(int ply, int pv_idx, int next_pv_idx);
    Move generate_move_nopromo(Square from_sq, Square to_sq);
    Score quiescence(Score alpha, Score beta, int ply);
    Score search(int depth, int ply, Score alpha, Score beta, bool is_pv_node, bool null_move_allowed = true);
    Score search_root(int depth, Score alpha, Score beta);
    bool is_search_stopped();
    void order_moves(Move hash_move, int ply);
    Score eval_pawns();
    Score eval_knights();
    Score eval_bishops();
    Score eval_rooks();
    Score eval_queens();
    Score eval_kings();

public:
    SearchParams search_params;
    BoardState state;
    std::array<Move, PV_TABLE_SIZE> pv_table = { nullmove };

    Board() {
        move_generator::init_sliding_move_tables();
        zobrist::init_keys();
        state.reset();
        prev_states.push_back(state);
    }

    Board(std::string fen) {
        move_generator::init_sliding_move_tables();
        zobrist::init_keys();
        load_fen(fen);
        prev_states.push_back(state);
    }

    Board(bool load_start) {
        if (!load_start) return;
        move_generator::init_sliding_move_tables();
        zobrist::init_keys();
        load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        prev_states.push_back(state);
    }

    void load_fen(std::string fen);
    void print_board();

    template <bool GEN_CAPTURES>
    [[gnu::hot]]
    void generate_moves();
    bool is_side_in_check(Colour side);
    void make_null_move();
    [[gnu::hot]]
    void make_move(Move move);
    [[gnu::hot]]
    void unmake_last_move();
    Score eval();
    void run_search();
    bool is_rep();
    void clean_search();
};

#endif