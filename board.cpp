#include "../include/board.h"
#include "../include/utils.h"
#include <cstring>
#include <iostream>

// Clears board flags and bitboards, then resets flags to defaults
void Board::reset() {
    std::fill(std::begin(bitboards), std::end(bitboards), 0ULL);
    std::fill(std::begin(piece_list), std::end(piece_list), Pieces::no_piece);
    Board::enpassant_square = Squares::no_square;
    Board::side_to_move = Colours::no_colour;
    Board::castling_rights = 0;
    Board::halfmove_clock = 0;
    Board::fullmove_counter = 1;
}

void Board::load_fen(std::string fen) {
    Board::reset();
    char c;
    int piece_idx = Pieces::no_piece;
    int sq = 0;
    int fen_idx = 0;
    while (sq < 64) {
        c = fen.at(fen_idx);
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            piece_idx = char_to_piece(c);
            int csq = sq ^ 56;
            piece_list[csq] = static_cast<Pieces>(piece_idx);
            bitboards[piece_idx] |= mask(static_cast<Squares>(csq));
            sq++;
        }

        else if (c == '/') {
            int rank = sq >> 3;
            sq = 8*rank;
        }

        else if (c > '0' && c < '9') {
            int empty_squares = c - '0';
            sq += empty_squares;
        }

        fen_idx++;
    }

    fen_idx++;
    c = fen.at(fen_idx);
    if (c == 'w')
        side_to_move = Colours::white;
    else
        side_to_move = Colours::black;

    fen_idx += 2;
    if (fen.at(fen_idx) == 'K')
        castling_rights |= CastlingRights::wking_side;
    if (fen.at(fen_idx + 1) == 'Q')
        castling_rights |= CastlingRights::wqueen_side;
    if (fen.at(fen_idx + 2) == 'k')
        castling_rights |= CastlingRights::bking_side;
    if (fen.at(fen_idx + 3) == 'q')
        castling_rights |= CastlingRights::bqueen_side;

    fen_idx += 5;
    if (fen.at(fen_idx) != '-') {
        int file = fen.at(fen_idx) - 'a';
        int rank = fen.at(fen_idx + 1) - '1';
        enpassant_square = static_cast<Squares>(file + 8*rank);
        fen_idx++;
    } 
    
    else
        enpassant_square = Squares::no_square;
    
    fen_idx += 2;
    if (fen_idx < fen.length()) {
        size_t end_idx;
        halfmove_clock = std::stoi(fen.substr(fen_idx), &end_idx);
        fen_idx += end_idx;
        fullmove_counter = std::stoi(fen.substr(fen_idx));
    }

    bitboards[12] = bitboards[p] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q] | bitboards[k];
    bitboards[13] = bitboards[P] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | bitboards[K];
    bitboards[14] = bitboards[12] | bitboards[13];
}

void Board::print_board() {
    
    for (int r = 7; r >= 0; r--) { // Loop over the ranks
        std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;
        for (int f = 0; f < 8; f++) { // Loop over the files
            char piece = ' '; // What piece to print, is empty
            if (piece_list[8*r+f] != Pieces::no_piece) // If there is a piece on this square
                piece = piece_to_char(piece_list[8*r+f]); // Set the piece to be printed

            std::cout << "| " << piece << " ";

        }

        std::cout << "| " << r+1 << std::endl;
    }

    // Print the board state flags

    std::cout << "+---+---+---+---+---+---+---+---+\n  a   b   c   d   e   f   g   h" << std::endl;
    std::cout << "Side to move: " << ((side_to_move == Colours::white) ? "White" : "Black") << std::endl;
    std::cout << "Castling rights: " << static_cast<int>(castling_rights) << std::endl;
    std::cout << "Enpassant square: " << ((enpassant_square == Squares::no_square) ? "-" : std::to_string(static_cast<int>(enpassant_square))) << std::endl;
    std::cout << "Halfmove clock: " << halfmove_clock << std::endl;
    std::cout << "Fullmove counter: " << fullmove_counter << std::endl;
}