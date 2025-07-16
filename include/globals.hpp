#ifndef GLOBALS_HPP_INCLUDE
#define GLOBALS_HPP_INCLUDE

#include <cstdint>

using BB = uint64_t;
using Key = uint64_t;
using Square = int;
using Piece = int;
using Colour = int;
using Move = uint16_t;
using Code = int;
using Score = int;

constexpr int nullmove = 0;

/// @brief encoded squares in 'Little-Endian Rank-File Mapping' format.
/// See https://www.chessprogramming.org/Square_Mapping_Considerations#Little-Endian_File-Rank_Mapping
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
enum PiecesEncoding : Piece {
    p, n, b, r, q, k, P, N, B, R, Q, K, bpieces = 12, wpieces, allpieces, no_piece
};

/// @brief Colour representation, also used for indexing.
enum ColoursEncoding : Colour {
    black = 0, white = 1, no_colour = 2
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

enum MoveCode : Code {
    quiet, dbpush, kcastle, qcastle, capture, epcapture, 
    npromo = 8, bpromo, rpromo, qpromo, c_npromo, c_bpromo, c_rpromo, c_qpromo
};

#endif