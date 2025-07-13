#ifndef EVAL_HPP_INCLUDE
#define EVAL_HPP_INCLUDE

#include "../include/board.hpp"
#include "../include/misc.hpp"
#include <array>

using namespace std;
using namespace bitboard_utils;

constexpr array<Score, 5> material = { 100, 320, 330, 500, 900 };

constexpr array<Score, 64> pawn_psqt = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    2, -5,-10, -5, -5, -7, -5,  2,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

constexpr array<Score, 64> knight_psqt = {
    -50,-20,-30,-30,-30,-30,-20,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  7, 13, 10, 10, 13,  7,-30,
    -30,  2, 12, 17, 17, 12,  2,-30,
    -30,  0, 12, 17, 17, 12,  0,-30,
    -30,  7, 13, 10, 10, 13,  7,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-20,-30,-30,-30,-30,-20,-50
};

constexpr array<Score, 64> bishop_psqt = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 12, 10, 10, 12,  0,-10,
    -10, 10,  7, 12, 12,  7, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

constexpr array<Score, 64> rook_psqt = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,   0,  5,  5,  0,  0,  0
};

constexpr array<Score, 64> queen_psqt = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

constexpr array<array<Score, 64>, 2> king_psqt = {{
    {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20, -5, -5, -5, -5, 20, 20,
        20, 30, 10,  0,  0,  7, 27, 17
    },
    {
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
    }
}};

constexpr Score double_pawns_pen = -10;
constexpr Score tripled_pawns_pen = -12;

constexpr Score passed_pawn_bonus = 17;

constexpr Score isolated_pawns_pen = -10;
constexpr Score half_isolated_pawns_pen = -4;

constexpr Score open_file_rooks_bonus = 10;
constexpr Score half_open_file_rooks_bonus = 5;

inline BB north_fill(BB gen) {
    gen |= (gen << 8);
    gen |= (gen << 16);
    gen |= (gen << 32);
    return gen;
}

inline BB south_fill(BB gen) {
    gen |= (gen >> 8);
    gen |= (gen >> 16);
    gen |= (gen >> 32);
    return gen;
}

inline BB file_fill(BB gen) { return north_fill(gen) | south_fill(gen); }

inline BB wfront_span(BB wpawns) { return north_fill(wpawns) << 8; }
inline BB brear_span(BB bpawns) { return north_fill(bpawns) << 8; }
inline BB bfront_span(BB bpawns) { return south_fill(bpawns) >> 8; }
inline BB wrear_span(BB wpawns) { return south_fill(wpawns) >> 8; }

inline BB wpassed_pawns(BB wpawns, BB bpawns) {
    BB allFrontSpans = bfront_span(bpawns);
    allFrontSpans |= shift_one(allFrontSpans, east)
                    |  shift_one(allFrontSpans, west);
    return wpawns & ~allFrontSpans;
}

inline BB bpassed_pawns(BB bpawns, BB wpawns) {
    BB allFrontSpans = wfront_span(wpawns);
    allFrontSpans |= shift_one(allFrontSpans, east)
                    |  shift_one(allFrontSpans, west);
    return bpawns & ~allFrontSpans;
}

inline BB no_neighbor_east_file(BB pawns) {
    return pawns & ~shift_one(file_fill(pawns), west);
}

inline BB no_neighbor_west_file(BB pawns) {
    return pawns & ~shift_one(file_fill(pawns), east);
}

inline BB isolanis(BB pawns) {
   return  no_neighbor_east_file(pawns)
         & no_neighbor_west_file(pawns);
}

inline BB half_isolanis(BB pawns) {
   return  no_neighbor_east_file(pawns)
         ^ no_neighbor_west_file(pawns);
}

inline BB open_file(BB wpanws, BB bpawns) {
   return ~file_fill(wpanws) & ~file_fill(bpawns);
}

inline BB half_open_or_open(BB gen) { return ~file_fill(gen); }

inline BB w_half_open_files(BB wpawns, BB bpawns) {
   return half_open_or_open(wpawns)
        ^ open_file(wpawns, bpawns);
}

inline BB b_half_open_files(BB wpawns, BB bpawns) {
   return half_open_or_open(bpawns)
        ^ open_file(wpawns, bpawns);
}


#endif