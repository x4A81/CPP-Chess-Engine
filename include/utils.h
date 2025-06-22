#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

#include "board.h"
#include <cstring>
#include <cstdint>

// returns the algebraic notation of a piece
char piece_to_char(Pieces piece);

// Does the reverse
Pieces char_to_piece(char c);

// Prints a bitboard in a chess board like format
void print_bitboard(BB bb);

void print_move(Move move);

std::string sqauare_to_algebraic(Squares sq);

#endif