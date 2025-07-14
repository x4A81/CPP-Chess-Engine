#include "../include/book.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>

using namespace polyglot;

bool poly_enpassant_available(BoardState& state) {
    Square sq_with_pawn = 0;
    Piece target_piece = state.side_to_move == white ? P : p;
    if (state.enpassant_square != no_square) {
        if (state.side_to_move == white)
            sq_with_pawn = state.enpassant_square + 8;
        else
            sq_with_pawn = state.enpassant_square - 8;
        if (state.piece_list[sq_with_pawn + 1] == target_piece
            || state.piece_list[sq_with_pawn - 1] == target_piece)
            return true;
    }

    return false;
}

PolyKey polyglot::gen_poly_key(BoardState& state) {
    PolyKey key = 0;
    for (Square sq = 0; sq < 64; sq++) {
        Piece piece = state.piece_list[sq];
        if (piece > bpieces) continue;
        piece = piece_to_poly_piece[piece];
        key ^= polyglot_keys[64 * piece + 8 * get_rank(sq) + get_file(sq)];
    }

    int offset = 768;
    if (state.castling_rights & wking_side)
        key ^= polyglot_keys[offset];

    if (state.castling_rights & wqueen_side)
        key ^= polyglot_keys[offset + 1];

    if (state.castling_rights & bking_side)
        key ^= polyglot_keys[offset + 2];

    if (state.castling_rights & bqueen_side)
        key ^= polyglot_keys[offset + 3];

    offset = 772;
    if (poly_enpassant_available(state))
        key ^= polyglot_keys[get_file(state.enpassant_square)];

    offset = 780;
    if (state.side_to_move == white)
        key ^= polyglot_keys[offset];
    
    return key;
}

std::vector<BookEntry> polyglot::probe_book(PolyKey key) {
    std::ifstream file(BOOK_PATH, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open book file: " << BOOK_PATH << std::endl;
        return {};
    }

    std::vector<BookEntry> results;
    BookEntry entry;

    while (file.read(reinterpret_cast<char*>(&entry), sizeof(BookEntry))) {
        // Convert from big-endian to host endianness
        PolyKey entry_key = __builtin_bswap64(entry.key);
        if (entry_key == key) {
            entry.key = entry_key;
            entry.move = __builtin_bswap16(entry.move);
            entry.weight = __builtin_bswap16(entry.weight);
            entry.learn = __builtin_bswap32(entry.learn);
            results.push_back(entry);
        }
    }

    return results;
}

Move polyglot::get_book_move(BookEntry& entry, BoardState& state) {
    int from = (entry.move >> 6) & 0x3F;
    int to   = entry.move & 0x3F;
    int promo = (entry.move >> 12) & 0x7;

    Code code = quiet;
    Piece moved = state.piece_list[from];
    Piece captured = state.piece_list[to];
    bool is_pawn = (moved == P || moved == p);
    bool is_white = moved < bpieces;

    // Detect promotion rank
    bool is_promotion_rank = (to >= 0 && to <= 7) || (to >= 56 && to <= 63);

    // Handle promotions (polyglot encodes them as promo piece in bits 12–14)
    if (promo && is_pawn && is_promotion_rank) {
        switch (promo) {
            case 1: code = is_white
                            ? (captured != no_piece ? c_npromo : npromo)
                            : (captured != no_piece ? c_npromo : npromo);
                    break;
            case 2: code = is_white
                            ? (captured != no_piece ? c_bpromo : bpromo)
                            : (captured != no_piece ? c_bpromo : bpromo);
                    break;
            case 3: code = is_white
                            ? (captured != no_piece ? c_rpromo : rpromo)
                            : (captured != no_piece ? c_rpromo : rpromo);
                    break;
            case 4: code = is_white
                            ? (captured != no_piece ? c_qpromo : qpromo)
                            : (captured != no_piece ? c_qpromo : qpromo);
                    break;
            default: code = quiet;
        }
    }

    // Castling (note: no need to check castling rights — book assumes legal move)
    else if (moved == K && from == e1 && to == g1) code = kcastle;
    else if (moved == K && from == e1 && to == c1) code = qcastle;
    else if (moved == k && from == e8 && to == g8) code = kcastle;
    else if (moved == k && from == e8 && to == c8) code = qcastle;

    // En passant (square is empty, but pawn moves to enpassant square)
    else if (is_pawn && to == state.enpassant_square && captured == no_piece)
        code = epcapture;

    // Double pawn push
    else if (is_pawn && std::abs(to - from) == 16 && captured == no_piece)
        code = dbpush;

    // Capture
    else if (captured != no_piece)
        code = capture;

    // Quiet
    else
        code = quiet;

    return (from) | (to << 6) | (code << 12);
}