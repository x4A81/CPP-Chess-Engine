#include "../include/utils.h"
#include "utils.h"
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
    printf("\nBitboard val: %lu \n", bb);
    for (int r = 7; r >= 0 ; r--) {
        printf(" %d |", r+1);
        for (int f = 0; f < 8; f++) {
            int sq = 8*r+f;
            std::cout << " " << (get_bit(bb, static_cast<Squares>(sq)) ? 1 : 0) << " ";
        }
    
        std::cout << std::endl;
    }
  
    std::cout << "     a  b  c  d  e  f  g  h" << std::endl;
}