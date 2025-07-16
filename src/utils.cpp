#include <print>

#include "../include/utils.hpp"
#include "../include/bitboard_math.hpp"
#include "../include/board.hpp"

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
    std::print("Bitboard val: {}\n", bb);
    for (int r = 7; r >= 0; --r) {
        std::print(" {} |", r+1);
        for (int f = 0; f < 8; ++f) {
            int sq = 8*r+f;
            std::print(" {} ", (bb_math::get_bit(bb, sq) ? 1 : 0));
        }
    
        std::print("\n");
    }
  
    std::println("     a  b  c  d  e  f  g  h");
}

void print_piece_list(std::array<Piece, 64> list) {
    for (int r = 7; r >= 0; --r) {
        std::println("+---+---+---+---+---+---+---+---+");
        for (int f = 0; f < 8; ++f) {
            char piece = ' ';
            if (list[8 * r + f] != no_piece)
                piece = piece_to_char(list[8 * r + f]);

            std::print("| {} ", piece);
        }

        std::println("| {}", r + 1);
    }

    std::println("+---+---+---+---+---+---+---+---+\n  a   b   c   d   e   f   g   h");
}

std::string square_to_string(Square sq) {
    char file = 'a' + get_file(sq);
    char rank = '1' + get_rank(sq);
    return std::string() + file + rank;
}

std::string move_to_string(const Move move) {
    Square from_sq = get_from_sq(move);
    Square to_sq = get_to_sq(move);
    Code code = get_code(move);

    std::string result = square_to_string(from_sq) + square_to_string(to_sq);

    switch (code) {
        case npromo:
        case c_npromo: result += 'n'; break;
        case bpromo:
        case c_bpromo: result += 'b'; break;
        case rpromo:
        case c_rpromo: result += 'r'; break;
        case qpromo:
        case c_qpromo: result += 'q'; break;
    }

    return result;
}