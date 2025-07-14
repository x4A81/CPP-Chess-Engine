#include "../include/utils.hpp"
#include "../include/misc.hpp"
#include <iostream>

char piece_to_char(Piece piece) {
    std::string piece_chars = "pnbrqkPNBRQK";
    return piece_chars.at(piece);
}

Piece char_to_piece(char c) {
    switch (c) {
        case 'p': return p;
        case 'n': return n;
        case 'b': return b;
        case 'r': return r;
        case 'q': return q;
        case 'k': return k;
        case 'P': return P;
        case 'N': return N;
        case 'B': return B;
        case 'R': return R;
        case 'Q': return Q;
        case 'K': return K;
        default:  return no_piece;
    }
}

void print_bitboard(BB bb) {
    printf("\nBitboard val: 0x%lX\n", bb);
    for (int r = 7; r >= 0 ; r--) {
        std::cout << " " << r+1 << " |";
        for (int f = 0; f < 8; f++) {
            int sq = 8*r+f;
            std::cout << " " << (bitboard_utils::get_bit(bb, sq) ? 1 : 0) << " ";
        }
    
        std::cout << std::endl;
    }
  
    std::cout << "     a  b  c  d  e  f  g  h" << std::endl;
}

void print_piece_list(std::array<Piece, 64> list) {
    for (int r = 7; r >= 0; r--) { // Loop over the ranks
        cout << "+---+---+---+---+---+---+---+---+" << endl;
        for (int f = 0; f < 8; f++) { // Loop over the files
            char piece = ' '; // What piece to print, is empty
            if (list[8*r+f] != no_piece) // If there is a piece on this square
                piece = piece_to_char(list[8*r+f]); // Set the piece to be printed

            cout << "| " << piece << " ";

        }

        cout << "| " << r+1 << endl;
    }

    cout << "+---+---+---+---+---+---+---+---+\n  a   b   c   d   e   f   g   h" << endl;
}

std::string square_to_string(Square sq) {
    char file = 'a' + get_file(sq);
    char rank = '1' + get_rank(sq);
    return std::string() + file + rank;
}

void print_move(Move move) {
    Square from_sq = get_from_sq(move);
    Square to_sq = get_to_sq(move);
    Code code = get_code(move);
    std::cout << square_to_string(from_sq) << square_to_string(to_sq);
    switch (code) {
        case npromo:
        case c_npromo: std::cout << 'n'; break;
        case bpromo:
        case c_bpromo: std::cout << 'b'; break;
        case rpromo:
        case c_rpromo: std::cout << 'r'; break;
        case qpromo:
        case c_qpromo: std::cout << 'q'; break;
    }
}