#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

#include "board.h"
#include <cstdint>

constexpr inline BB mask(Squares square) { return (1ULL << static_cast<int>(square)); }
constexpr inline void set_bit(BB &bitboard, Squares square) { bitboard |= mask(square); }
constexpr inline BB get_bit(BB bitboard, Squares square) { return bitboard & mask(square); }

// returns the algebraic notation of a piece
char piece_to_char(Pieces piece);

// Does the reverse
Pieces char_to_piece(char c);

// Prints a bitboard in a chess board like format
void print_bitboard(BB bb);

#endif