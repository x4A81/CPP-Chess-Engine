#include "../include/utils.h"
#include "../include/bitboard_utils.h"
#include <iostream>

char piece_to_char(Pieces piece) {
    std::string piece_chars = "pnbrqkPNBRQK";
    return piece_chars.at(static_cast<int>(piece));
}

Pieces char_to_piece(char c) {
    switch (c) {
        case 'P': return Pieces::P;
        case 'N': return Pieces::N;
        case 'B': return Pieces::B;
        case 'R': return Pieces::R;
        case 'Q': return Pieces::Q;
        case 'K': return Pieces::K;
        case 'p': return Pieces::p;
        case 'n': return Pieces::n;
        case 'b': return Pieces::b;
        case 'r': return Pieces::r;
        case 'q': return Pieces::q;
        case 'k': return Pieces::k;
        default:  return Pieces::no_piece;
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

std::string sqauare_to_algebraic(Squares sq) {
    char file = 'a' + (sq & 7);
    char rank = '1' + (sq >> 3);
    return std::string() + file + rank;
}

void print_move(Move move) {
    int from_sq = move & 0b111111;
    int to_sq = (move >> 6) & 0b111111;
    std::cout << sqauare_to_algebraic(Squares(from_sq)) << sqauare_to_algebraic(Squares(to_sq));
    int code = move >> 12;
    switch (code) {
        case 8:
        case 12: std::cout << 'n';
        case 9:
        case 13: std::cout << 'b';
        case 10:
        case 14: std::cout << 'r';
        case 11:
        case 15: std::cout << 'q';
    }
}