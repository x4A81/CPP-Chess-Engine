#ifndef UTILS_HPP_INCLUDE
#define UTILS_HPP_INCLUDE

#include <cstring>
#include <cstdint>

#include "globals.hpp"

/// @brief Prints a bitboard in a chess-board format
/// @param bb the bitboard to print
void print_bitboard(BB bb);

/// @brief Prints a piece-list in a chess-board format
/// @param list the piece-list to print
void print_piece_list(std::array<Piece, 64> list);

/// @brief Converts a value in the Pieces enum into a character in pure coordinate notation
/// @param piece the value to convert
/// @return the notation of the piece
char piece_to_char(Piece piece);

/// @brief Converts pure coordinate notation of a piece into a value in the Pieces enum
/// @param c notation
/// @return the value of the piece
Piece char_to_piece(char c);

/// @brief Prints an encoded move in 'Pure coordinate notation'
/// @param move the encoded move to print
void print_move(Move move);

/// @brief Converts a square value in the Squares enum into pure coordinate notation
/// @param sq 
/// @return the notation of the square
std::string square_to_string(Square sq);

/// @note For more on Pure coordinate notation, see https://www.chessprogramming.org/Algebraic_Chess_Notation#Pure_coordinate_notation

#endif