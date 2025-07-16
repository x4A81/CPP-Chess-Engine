#include <random>
#include "../include/board.hpp"

#define SEED 34567

void zobrist::init_keys() {
    std::mt19937_64 gen(Key(SEED)); 
    std::uniform_int_distribution<Key> dist(0, UINT64_MAX);

    for (auto& key : piece_keys) {
        key = dist(gen);
    }

    for (auto& key : castling_keys) {
        key = dist(gen);
    }

    for (auto& key : ep_file_key) {
        key = dist(gen);
    }

    side_key = dist(gen);
}

Key zobrist::gen_pos_key(BoardState& state) {
    Key key = Key(0);
    for (Square sq = 0; sq < 64; ++sq) {
        Piece piece = state.piece_list.at(sq);
        if (piece > bpieces) continue;
        key ^= piece_keys.at(piece * 64 + sq);
    }

    if (state.enpassant_square != no_square) {
        int file = state.enpassant_square & 7;
        key ^= ep_file_key.at(file);
    }

    if (state.castling_rights & wking_side)
        key ^= castling_keys.at(0);
    if (state.castling_rights & wqueen_side)
        key ^= castling_keys.at(1);
    if (state.castling_rights & bking_side)
        key ^= castling_keys.at(2);
    if (state.castling_rights & bqueen_side)
        key ^= castling_keys.at(3);

    if (state.side_to_move == black)
        key ^= side_key;

    return key;
}