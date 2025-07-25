#include <print>
#include <sstream>

#include "../include/board.hpp"
#include "../include/utils.hpp"
#include "../include/book.hpp"

using namespace bb_math;
using namespace move_generator;

// Clears board flags and bitboards, then resets flags to defaults
void BoardState::reset() {
    std::fill(begin(bitboards), end(bitboards), 0ULL);
    std::fill(begin(piece_list), end(piece_list), no_piece);
    enpassant_square = no_square;
    side_to_move = no_colour;
    castling_rights = 0;
    halfmove_clock = 0;
    fullmove_counter = 1;
    hash_key = 0;
}

void Board::load_fen(std::string fen) {
    state.reset();

    std::istringstream ss(fen);
    std::string board_part, stm, castling, enpassant, halfmove, fullmove;

    ss >> board_part >> stm >> castling >> enpassant >> halfmove >> fullmove;

    Square sq = 0;
    for (char c : board_part) {
        if (isdigit(c)) {
            sq += c - '0';
        } else if (c == '/') {
            continue;
        } else {
            Piece piece_idx = char_to_piece(c);
            Square flip_sq = flip_rank(sq);  
            state.piece_list[flip_sq] = piece_idx;
            state.bitboards[piece_idx] |= mask(flip_sq);
            sq++;
        }
    }

    state.side_to_move = (stm == "w") ? white : black;

    state.castling_rights = 0;
    if (castling != "-") {
        for (char c : castling) {
            switch (c) {
                case 'K': state.castling_rights |= wking_side; break;
                case 'Q': state.castling_rights |= wqueen_side; break;
                case 'k': state.castling_rights |= bking_side; break;
                case 'q': state.castling_rights |= bqueen_side; break;
            }
        }
    }

    if (enpassant != "-") {
        int file = enpassant[0] - 'a';
        int rank = enpassant[1] - '1';
        state.enpassant_square = file + 8 * rank;
    } else {
        state.enpassant_square = no_square;
    }

    state.halfmove_clock = stoi(halfmove);
    state.fullmove_counter = stoi(fullmove);

    state.bitboards[12] = state.bitboards[p] | state.bitboards[n] | state.bitboards[b] |
                          state.bitboards[r] | state.bitboards[q] | state.bitboards[k];
    state.bitboards[13] = state.bitboards[P] | state.bitboards[N] | state.bitboards[B] |
                          state.bitboards[R] | state.bitboards[Q] | state.bitboards[K];
    state.bitboards[14] = state.bitboards[12] | state.bitboards[13];

    state.hash_key = zobrist::gen_pos_key(state);
}

void Board::print_board() {
    
    for (int r = 7; r >= 0; --r) {
        std::print("+---+---+---+---+---+---+---+---+\n"); 
        for (int f = 0; f < 8; ++f) {
            char piece = ' ';
            if (state.piece_list[8*r+f] != no_piece)
                piece = piece_to_char(state.piece_list[8*r+f]); 

            std::print("| {} ", piece);
        }

        std::print("| \n");
    }

    // Print the board state vars

    std::print(
    "+---+---+---+---+---+---+---+---+\n"
    "  a   b   c   d   e   f   g   h\n"
    );

    std::print("Side to move: {}\n",
        (state.side_to_move == white) ? "White" : "Black"
    );

    std::print("Castling rights: {}{}{}{}\n",
        (state.castling_rights & wking_side) ? "K" : "-",
        (state.castling_rights & wqueen_side) ? "Q" : "-",
        (state.castling_rights & bking_side) ? "k" : "-",
        (state.castling_rights & bqueen_side) ? "q" : "-"
    );

    std::print("Enpassant square: {}\n",
        (state.enpassant_square == no_square) ? "-" : square_to_string(state.enpassant_square)
    );

    std::print("Halfmove clock: {}\n", state.halfmove_clock);
    std::print("Fullmove counter: {}\n", state.fullmove_counter);
    std::print("Hash Key: {}\n", state.hash_key);
    std::print("Poly Key: {}\n", polyglot::gen_poly_key(state));

    std::println("Checked: {}",
        is_side_in_check(state.side_to_move) ? "True" : "No"
    );
}

[[gnu::hot]]
Move Board::generate_move_nopromo(Square from_sq, Square to_sq) {
    Move move = from_sq | (to_sq << 6);
    Code code = 0;
    Piece piece = state.piece_list[from_sq];
    bool is_pawn = (piece == P || piece == p);
    
    if (is_pawn && (to_sq - from_sq == 16 || to_sq - from_sq == -16))
        code = dbpush;

    if (piece == K && from_sq == e1) {
        if (to_sq == g1) code = kcastle;
        if (to_sq == c1) code = qcastle;
    }

    if (piece == k && from_sq == e8) {
        if (to_sq == g8) code = kcastle;
        if (to_sq == c8) code = qcastle;
    }

    if (state.piece_list[to_sq] != no_piece)
        code = capture;

    if ((to_sq == state.enpassant_square) && is_pawn)
        code = epcapture;

    return move | (code << 12);
}

BB Board::get_attacked_BB(Colour side) {
    BB white_mask = -(side == white); // All 1s if white
    BB black_mask = ~white_mask;

    BB wh = state.bitboards[wpieces];
    BB bl = state.bitboards[bpieces];
    BB occ = state.bitboards[allpieces];

    BB opponent_pieces = (bl & white_mask) | (wh & black_mask); // isolates opponent pieces

    // Check mask
    BB friendly_pieces = (bl & black_mask) | (wh & white_mask);
    BB occ_ex_king = ((state.bitboards[k] | state.bitboards[K]) & ~opponent_pieces) ^ occ;
    BB opp_any_attacks = 0;
    Square king_sq = bitscan_forward((state.bitboards[k] | state.bitboards[K]) & friendly_pieces);
    BB opp_attacks = 0;

    BB opp_rooks = (state.bitboards[r] | state.bitboards[R] | state.bitboards[q] | state.bitboards[Q]) & opponent_pieces;
    
    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, west);
    opp_any_attacks |= opp_attacks;

    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, east);
    opp_any_attacks |= opp_attacks;

    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, nort);
    opp_any_attacks |= opp_attacks;

    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, sout);
    opp_any_attacks |= opp_attacks;
    
    BB opp_bishops = (state.bitboards[b] | state.bitboards[B] | state.bitboards[q] | state.bitboards[Q]) & opponent_pieces;
    
    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, noEast);
    opp_any_attacks |= opp_attacks;

    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, soWest);
    opp_any_attacks |= opp_attacks;

    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, soEast);
    opp_any_attacks |= opp_attacks;

    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, noWest);
    opp_any_attacks |= opp_attacks;

    // Non-sliding pieces
    opp_any_attacks |= knight_attacks((state.bitboards[n] | state.bitboards[N]) & opponent_pieces);
    opp_any_attacks |= king_attacks((state.bitboards[k] | state.bitboards[K]) & opponent_pieces);
    
    if (state.side_to_move == white)
        opp_any_attacks |= bpawn_attacks(state.bitboards[p]);
    else
        opp_any_attacks |= wpawn_attacks(state.bitboards[P]);

    return opp_any_attacks;
}

BB Board::sq_attacked_by(BB occ, Square sq) {
    BB knights, kings, bishopsQueens, rooksQueens;
    knights        = state.bitboards[n] | state.bitboards[N];
    kings          = state.bitboards[k] | state.bitboards[K];
    rooksQueens    = state.bitboards[q] | state.bitboards[Q];
    bishopsQueens  = state.bitboards[q] | state.bitboards[Q];
    rooksQueens   |= state.bitboards[r] | state.bitboards[R];
    bishopsQueens |= state.bitboards[b] | state.bitboards[B];

    return (pawn_attack_table[sq][white] & state.bitboards[p])
            | (pawn_attack_table[sq][black] & state.bitboards[P])
            | (knight_attacks(mask(sq)) & knights)
            | (king_move_table[sq] & kings)
            | (bishop_moves(sq, occ) & bishopsQueens)
            | (rook_moves(sq, occ) & rooksQueens)
            | (x_ray_bishop(occ, sq) & bishopsQueens)
            | (x_ray_rook(occ, sq) & rooksQueens)
            ;
}

bool Board::is_side_in_check(Colour side) {
    return state.bitboards[k + (side == white ? 6 : 0)] & get_attacked_BB(side);
}

template <bool GEN_CAPTURES>
[[gnu::hot]]
void Board::generate_moves() {

    // Handle the movement mask
    BB move_mask = ~0;
    BB white_mask = -(state.side_to_move == white); // All 1s if white to move, else 0s
    BB black_mask = ~white_mask;

    BB wh = state.bitboards[wpieces];
    BB bl = state.bitboards[bpieces];
    BB occ = state.bitboards[allpieces];

    BB opponent_pieces = (bl & white_mask) | (wh & black_mask); // isolates opponent pieces

    if constexpr (GEN_CAPTURES)
        move_mask = opponent_pieces;

    // Check mask
    BB friendly_pieces = (bl & black_mask) | (wh & white_mask);
    BB hor_inbetween = 0, ver_inbetween = 0, dia_inbetween = 0, antdia_inbetween = 0;
    BB occ_ex_king = ((state.bitboards[k] | state.bitboards[K]) & ~opponent_pieces) ^ occ;
    BB opp_any_attacks = 0;
    BB kingBB = (state.bitboards[k] | state.bitboards[K]) & friendly_pieces;
    Square king_sq = bitscan_forward(kingBB);
    BB king_super_dia = bishop_moves(king_sq, occ);
    BB king_super_orth = rook_moves(king_sq, occ);
    BB opp_attacks = 0;

    BB opp_rooks = (state.bitboards[r] | state.bitboards[R] | state.bitboards[q] | state.bitboards[Q]) & opponent_pieces;
    
    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, west);
    opp_any_attacks |= opp_attacks;
    hor_inbetween |= opp_attacks & king_super_orth & precomp_east_fill[king_sq];

    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, east);
    opp_any_attacks |= opp_attacks;
    hor_inbetween |= opp_attacks & king_super_orth & precomp_west_fill[king_sq];

    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, nort);
    opp_any_attacks |= opp_attacks;
    ver_inbetween |= opp_attacks & king_super_orth & precomp_sout_fill[king_sq];

    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, sout);
    opp_any_attacks |= opp_attacks;
    ver_inbetween |= opp_attacks & king_super_orth & precomp_nort_fill[king_sq];
    
    BB opp_bishops = (state.bitboards[b] | state.bitboards[B] | state.bitboards[q] | state.bitboards[Q]) & opponent_pieces;
    
    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, noEast);
    opp_any_attacks |= opp_attacks;
    dia_inbetween |= opp_attacks & king_super_dia & precomp_soWest_fill[king_sq];

    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, soWest);
    opp_any_attacks |= opp_attacks;
    dia_inbetween |= opp_attacks & king_super_dia & precomp_noEast_fill[king_sq];;

    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, soEast);
    opp_any_attacks |= opp_attacks;
    antdia_inbetween |= opp_attacks & king_super_dia & precomp_noWest_fill[king_sq];

    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, noWest);
    opp_any_attacks |= opp_attacks;
    antdia_inbetween |= opp_attacks & king_super_dia & precomp_soEast_fill[king_sq];

    // Non-sliding pieces
    opp_any_attacks |= knight_attacks((state.bitboards[n] | state.bitboards[N]) & opponent_pieces);
    opp_any_attacks |= king_attacks((state.bitboards[k] | state.bitboards[K]) & opponent_pieces);
    BB kings = (state.bitboards[k] | state.bitboards[K]) & opponent_pieces;
    
    if (state.side_to_move == white)
        opp_any_attacks |= bpawn_attacks(state.bitboards[p]);
    else
        opp_any_attacks |= wpawn_attacks(state.bitboards[P]);

    
    BB all_inbetween = hor_inbetween | ver_inbetween | antdia_inbetween | dia_inbetween;
    BB block_mask = all_inbetween & ~occ;
    BB checkers_mask = (king_super_orth & opp_rooks) | (king_super_dia & opp_bishops) 
    | (knight_move_table[king_sq] & (state.bitboards[n] | state.bitboards[N]) & opponent_pieces)
    | (pawn_attack_table[king_sq][state.side_to_move] & (state.bitboards[p] | state.bitboards[P]) & opponent_pieces);

    int64_t null_if_check = (int64_t(opp_any_attacks & mask(king_sq)) - 1) >> 63;
    int64_t null_if_dbl_check = (int64_t(checkers_mask & (checkers_mask - 1)) - 1) >> 63;

    BB to_checkers_mask = checkers_mask | block_mask | null_if_check;
    move_mask &= ~friendly_pieces & to_checkers_mask & null_if_dbl_check;
    state.move_list.clear();
    
    BB king_movement = king_move_table[king_sq] & ~(friendly_pieces | opp_any_attacks);
    if constexpr (GEN_CAPTURES) king_movement &= opponent_pieces;
    while (king_movement) {
        Square to_sq = pop_lsb(king_movement);
        state.move_list.add(generate_move_nopromo(king_sq, to_sq));
    }

    state.is_in_check = null_if_check == 0;
    
    // If dbl check only king moves are allowed
    if (move_mask == 0) return;
    
    // Pinned knights cannot move
    BB knights = (state.bitboards[n] | state.bitboards[N]) & friendly_pieces & ~all_inbetween;
    while (knights) {
        Square from_sq = pop_lsb(knights);
        BB movement = knight_move_table[from_sq] & move_mask;
        while (movement) {
            Square to_sq = pop_lsb(movement);
            state.move_list.add(generate_move_nopromo(from_sq, to_sq));
        }
    }

    // Rook and Queen moves
    BB rooks = (state.bitboards[r] | state.bitboards[R] | state.bitboards[q] | state.bitboards[Q]) & friendly_pieces;

    while (rooks) {
        Square from_sq = pop_lsb(rooks);
        BB moves = move_mask & rook_moves(from_sq, occ);
        if (get_bit(all_inbetween, from_sq)) {
            if (get_bit(hor_inbetween, from_sq)) moves &= precomp_hor_fill[from_sq];
            else if (get_bit(ver_inbetween, from_sq)) moves &= precomp_ver_fill[from_sq];
            else if (get_bit(dia_inbetween | antdia_inbetween, from_sq)) moves &= 0;
        }

        while (moves) {
            Square to_sq = pop_lsb(moves);
            state.move_list.add(generate_move_nopromo(from_sq, to_sq));
        }
    }

    // Queen and bishop moves
    BB bishops = (state.bitboards[b] | state.bitboards[B] | state.bitboards[q] | state.bitboards[Q]) & friendly_pieces;

    while (bishops) {
        Square from_sq = pop_lsb(bishops);
        BB moves = move_mask & bishop_moves(from_sq, occ);
        if (get_bit(all_inbetween, from_sq)) {
            if (get_bit(dia_inbetween, from_sq)) moves &= precomp_dia_fill[from_sq];
            else if (get_bit(antdia_inbetween, from_sq)) moves &= precomp_antdia_fill[from_sq];
            else if (get_bit(hor_inbetween | ver_inbetween, from_sq)) moves &= 0;
        }

        while (moves) {
            Square to_sq = pop_lsb(moves);
            state.move_list.add(generate_move_nopromo(from_sq, to_sq));
        }
    }

    // Pawn Moves
    BB pawns = (state.bitboards[p] | state.bitboards[P]) & friendly_pieces  & ~(all_inbetween ^ ver_inbetween);
    BB pawn_push_mask = shift_one(pawns, Dir(int(sout) ^ state.side_to_move)) & ~occ;
    BB rank4 = 0x00000000FF000000ULL;
    BB rank5 = 0x000000FF00000000ULL;
    BB dbl_rank = (state.side_to_move == white) ? rank4 : rank5;
    while (pawns) {
        Square from_sq = pop_lsb(pawns);

        // Single push
        Square to_sq = from_sq - shifts[state.side_to_move];
        if (get_bit(pawn_push_mask & move_mask, to_sq)) {
            if (to_sq >= a8 || to_sq <= h1) {
                Move move_no_promo = generate_move_nopromo(from_sq, to_sq);
                state.move_list.add((npromo << 12) | move_no_promo);
                state.move_list.add((bpromo << 12) | move_no_promo);
                state.move_list.add((rpromo << 12) | move_no_promo);
                state.move_list.add((qpromo << 12) | move_no_promo);
            } else
                state.move_list.add(generate_move_nopromo(from_sq, to_sq));
        }

        // Dbl push
        to_sq += shifts[state.side_to_move ^ 1];
        if (get_bit(shift_one(pawn_push_mask, Dir(int(sout) ^ state.side_to_move)) & dbl_rank & ~occ & move_mask, to_sq))
            state.move_list.add(generate_move_nopromo(from_sq, to_sq));
    }

    // Attacks
    BB ep_mask = state.enpassant_square == no_square ? 0 : mask(state.enpassant_square);
    BB targets = (opponent_pieces & move_mask) | ep_mask;
    pawns = (state.bitboards[p] | state.bitboards[P]) & friendly_pieces;

    while (pawns) {
        Square from_sq = pop_lsb(pawns);

        BB movement_mask = pawn_attack_table[from_sq][state.side_to_move] & targets;
        if (get_bit(all_inbetween, from_sq)) {
            if (get_bit(dia_inbetween, from_sq)) movement_mask &= precomp_dia_fill[from_sq];
            else if (get_bit(antdia_inbetween, from_sq)) movement_mask &= precomp_antdia_fill[from_sq];
            else if (get_bit(hor_inbetween | ver_inbetween, from_sq)) movement_mask &= 0;
        }
        while (movement_mask) {
            Square to_sq = pop_lsb(movement_mask);

            if (to_sq == state.enpassant_square) {
                if (null_if_check) {
                    BB c_occ = occ;
                    pop_bit(c_occ, from_sq);
                    pop_bit(c_occ, (state.side_to_move == white ? to_sq - 8 : to_sq + 8));
                    if (precomp_hor_fill[king_sq] & rook_moves(king_sq, c_occ) & opp_rooks) {
                        // enpassant is not legal
                        continue;
                    }
                } else
                    if (!(mask(state.enpassant_square + (state.side_to_move == white ? -8 : 8)) & to_checkers_mask)
                    && !(mask(state.enpassant_square) & to_checkers_mask)) continue;
            }

            if (to_sq >= a8 || to_sq <= h1) {
                Move move_no_promo = generate_move_nopromo(from_sq, to_sq);
                state.move_list.add((c_npromo << 12) | move_no_promo);
                state.move_list.add((c_bpromo << 12) | move_no_promo);
                state.move_list.add((c_rpromo << 12) | move_no_promo);
                state.move_list.add((c_qpromo << 12) | move_no_promo);
            } else
                state.move_list.add(generate_move_nopromo(from_sq, to_sq));
        }
    }

    // Caslting
    if (null_if_check && !GEN_CAPTURES) {
        if (state.side_to_move == white) {
            if (
                // Can castle
                (state.castling_rights & wking_side)
                
                // Squares king travel aren't attacked
                && ((opp_any_attacks & (mask(f1) | mask(g1))) == 0)
                
                // There are no pieces between king & rook
                
                && ((occ & (mask(f1) | mask(g1))) == 0)
                
            )
            state.move_list.add(generate_move_nopromo(e1, g1));

            if (
                // Can castle
                (state.castling_rights & wqueen_side)

                // Squares king travel aren't attacked
                && ((opp_any_attacks & (mask(d1) | mask(c1))) == 0)

                // There are no pieces between king & rook

                && ((occ & (mask(d1) | mask(c1) | mask(b1))) == 0)
            
            )
            state.move_list.add(generate_move_nopromo(e1, c1));

        } else {
            if (
                // Can castle
                (state.castling_rights & bking_side)

                // Squares king travel aren't attacked
                && ((opp_any_attacks & (mask(f8) | mask(g8))) == 0)
                
                // There are no pieces between king & rook
                
                && ((occ & (mask(f8) | mask(g8))) == 0)
            
            )
            state.move_list.add(generate_move_nopromo(e8, g8));

            if (
                // Can castle
                (state.castling_rights & bqueen_side)

                // Squares king travel aren't attacked
                && ((opp_any_attacks & (mask(d8) | mask(c8))) == 0)

                // There are no pieces between king & rook

                && ((occ & (mask(d8) | mask(c8) | mask(b8))) == 0)
            
            )
            state.move_list.add(generate_move_nopromo(e8, c8));
        }
    }

}

template void Board::generate_moves<CAPTURES>();
template void Board::generate_moves<ALLMOVES>();

void Board::make_null_move() {
    prev_state_idx++;
    prev_states[prev_state_idx] = state;
    if (state.side_to_move == black) state.fullmove_counter++;
    state.side_to_move = state.side_to_move ^ 1;
    state.hash_key ^= zobrist::side_key;
    state.enpassant_square = no_square;
    state.halfmove_clock = 0;
}

[[gnu::hot]]
void Board::make_move(Move move) {
    prev_state_idx++;
    prev_states[prev_state_idx] = state;
    Key& key = state.hash_key;
    Square from_sq = get_from_sq(move), to_sq = get_to_sq(move);
    Piece piece = state.piece_list[from_sq];
    Colour piece_colour = piece <= 5 ? bpieces : wpieces;
    Code move_code = get_code(move);

    bool white_tm = state.side_to_move == white;
    Colour c_piece_colour;
    Square ep = state.enpassant_square, cap_sq;
    key ^= zobrist::ep_file_key[get_file(ep)];
    Piece promo_piece, c_piece;
    state.enpassant_square = no_square;

    // Remove the captured piece.
    if (move_code == capture || move_code >= c_npromo) {
        c_piece = state.piece_list[to_sq];
        c_piece_colour = white_tm ? bpieces : wpieces;
        pop_bit(state.bitboards[c_piece], to_sq);
        pop_bit(state.bitboards[c_piece_colour], to_sq);
        key ^= zobrist::piece_keys[c_piece * 64 + to_sq];
    }

    // Remove the pawn.
    else if (move_code == epcapture) {
        cap_sq = white_tm ? ep - 8 : ep + 8;
        c_piece = white_tm ? p : P;
        c_piece_colour = white_tm ? bpieces : wpieces;
        pop_bit(state.bitboards[c_piece], cap_sq);
        pop_bit(state.bitboards[c_piece_colour], cap_sq);
        state.piece_list[cap_sq] = no_piece;
        key ^= zobrist::piece_keys[c_piece * 64 + cap_sq];
    }

    // Move the piece
    state.bitboards[piece] ^= mask(from_sq) | mask(to_sq);
    state.bitboards[piece_colour] ^= mask(from_sq) | mask(to_sq);
    state.piece_list[from_sq] = no_piece;
    state.piece_list[to_sq] = piece;
    key ^= zobrist::piece_keys[piece * 64 + from_sq];
    key ^= zobrist::piece_keys[piece * 64 + to_sq];

    // Update enpassant sq.
    if (move_code == dbpush) {
        state.enpassant_square = white_tm ? to_sq - 8 : to_sq + 8;
        key ^= zobrist::ep_file_key[get_file(state.enpassant_square)];
    }

    else if (move_code == kcastle) {
        // Move the rook.
        if (white_tm) {
            state.bitboards[R] ^= mask(h1) | mask(f1);
            state.bitboards[wpieces] ^= mask(h1) | mask(f1);
            key ^= zobrist::piece_keys[R * 64 + h1];
            key ^= zobrist::piece_keys[R * 64 + f1];
            state.piece_list[h1] = no_piece;
            state.piece_list[f1] = R;
        } else {
            state.bitboards[r] ^= mask(h8) | mask(f8);
            state.bitboards[bpieces] ^= mask(h8) | mask(f8);
            key ^= zobrist::piece_keys[r * 64 + h8];
            key ^= zobrist::piece_keys[r * 64 + f8];
            state.piece_list[h8] = no_piece;
            state.piece_list[f8] = r;
        }
    }

    else if (move_code == qcastle) {
        if (white_tm) {
            state.bitboards[R] ^= mask(a1) | mask(d1);
            state.bitboards[wpieces] ^= mask(a1) | mask(d1);
            key ^= zobrist::piece_keys[R * 64 + a1];
            key ^= zobrist::piece_keys[R * 64 + d1];
            state.piece_list[a1] = no_piece;
            state.piece_list[d1] = R;
        } else {
            state.bitboards[r] ^= mask(a8) | mask(d8);
            state.bitboards[bpieces] ^= mask(a8) | mask(d8);
            key ^= zobrist::piece_keys[r * 64 + a8];
            key ^= zobrist::piece_keys[r * 64 + d8];
            state.piece_list[a8] = no_piece;
            state.piece_list[d8] = r;
        }
    }

    else if (move_code >= npromo) {
        switch (move_code)
        {
            case c_npromo:
            case npromo: promo_piece = white_tm ? N : n; break;
            
            case c_bpromo:
            case bpromo: promo_piece = white_tm ? B : b; break;
            
            case c_rpromo:
            case rpromo: promo_piece = white_tm ? R : r; break;
            
            case c_qpromo:
            case qpromo: promo_piece = white_tm ? Q : q; break;
            default:
            break;
        }

        // Remove the pawn.
        pop_bit(state.bitboards[piece], to_sq);
        key ^= zobrist::piece_keys[piece * 64 + to_sq];

        // Update the occupancy.
        set_bit(state.bitboards[promo_piece], to_sq);
        state.piece_list[to_sq] = promo_piece;
        key ^= zobrist::piece_keys[promo_piece * 64 + to_sq];
    }

    // Castling rights
    CastlingRights from_castling = castle_encoder[from_sq];
    CastlingRights to_castling = castle_encoder[to_sq];
    if (from_castling != 15 || to_castling != 15) {

        if (state.castling_rights & wking_side) key ^= zobrist::castling_keys[0];
        if (state.castling_rights & wqueen_side) key ^= zobrist::castling_keys[1];
        if (state.castling_rights & bking_side) key ^= zobrist::castling_keys[2];
        if (state.castling_rights & bqueen_side) key ^= zobrist::castling_keys[3];
    
        state.castling_rights &= from_castling;
        state.castling_rights &= to_castling;
    
        if (state.castling_rights & wking_side) key ^= zobrist::castling_keys[0];
        if (state.castling_rights & wqueen_side) key ^= zobrist::castling_keys[1];
        if (state.castling_rights & bking_side) key ^= zobrist::castling_keys[2];
        if (state.castling_rights & bqueen_side) key ^= zobrist::castling_keys[3];
    }

    // Update counters and side to move
    if (state.side_to_move == black) state.fullmove_counter++;
    state.halfmove_clock = (move_code == capture || piece == p || piece == P) ? 0 : state.halfmove_clock + 1;

    state.side_to_move ^= 1;
    key ^= zobrist::side_key;

    // Recalculate all-piece sets
    state.bitboards[allpieces] = state.bitboards[bpieces] | state.bitboards[wpieces];
}

[[gnu::hot]]
void Board::unmake_last_move() {
    state = prev_states[prev_state_idx];
    prev_state_idx--;
}

bool Board::is_rep() {
    if (state.halfmove_clock >= 50) return true;

    int repetition_count = 1; // include current state
    Key current_key = state.hash_key;
    for (const auto& prev : prev_states) {
        if (prev.hash_key == current_key)
            repetition_count++;
    }

    if (repetition_count > 2) return true;

    return false;
}

BB Board::get_least_valuable_piece(BB attackdef, Colour side, Piece& piece) {
    Piece start = side == white ? P : p;
    Piece end = start + 6;
    for (piece = start; piece < end; ++piece) {
        BB sub = attackdef & state.bitboards[piece];
        if (sub) return sub & -sub;
    }

    return 0;
}

std::array<Score, 6> material = { 100, 320, 330, 500, 900, 1000};

Score Board::see(Square to_sq, Piece target, Square from_sq, Piece att_piece) {
    std::array<Score, 32> gain;
    int d = 0;
    BB fromBB = mask(from_sq);
    BB occ = state.bitboards[allpieces];
    BB attackdef = sq_attacked_by(occ, to_sq);
    Colour side = target < 6 ? white : black;
    gain[d] = material[target > 5 ? target - 6 : target];

    do {
        d++;
        gain[d] = material[att_piece > 5 ? att_piece - 6 : att_piece] - gain[d - 1];
        attackdef ^= fromBB;
        occ ^= fromBB;
        side ^= 1;
        fromBB = get_least_valuable_piece(attackdef, side, att_piece);
    } while (fromBB);

    while (--d)
        gain[d-1] = -std::max(-gain[d-1], gain[d]);
    
    return gain[0];
}