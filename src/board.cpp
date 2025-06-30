#include "../include/board.hpp"
#include "../include/utils.hpp"
#include "../include/misc.hpp"
#include <iostream>
#include <sstream>

using namespace bitboard_utils;
using namespace movegenerator;
using namespace std;

// Clears board flags and bitboards, then resets flags to defaults
void Board_State::reset() {
    fill(begin(bitboards), end(bitboards), 0ULL);
    fill(begin(piece_list), end(piece_list), Pieces::no_piece);
    enpassant_square = Squares::no_square;
    side_to_move = Colours::no_colour;
    castling_rights = 0;
    halfmove_clock = 0;
    fullmove_counter = 1;
    hash_key = 0;
}


void Board::load_fen(string fen) {
    state.reset();

    istringstream ss(fen);
    string board_part, stm, castling, enpassant, halfmove, fullmove;

    ss >> board_part >> stm >> castling >> enpassant >> halfmove >> fullmove;

    int sq = 0;
    for (char c : board_part) {
        if (isdigit(c)) {
            sq += c - '0';
        } else if (c == '/') {
            continue;
        } else {
            int piece_idx = char_to_piece(c);
            int csq = sq ^ 56;
            state.piece_list[csq] = Pieces(piece_idx);
            state.bitboards[piece_idx] |= bitboard_utils::mask(csq);
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
        state.enpassant_square = Squares(file + 8 * rank);
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
    
    for (int r = 7; r >= 0; r--) { // Loop over the ranks
        cout << "+---+---+---+---+---+---+---+---+" << endl;
        for (int f = 0; f < 8; f++) { // Loop over the files
            char piece = ' '; // What piece to print, is empty
            if (state.piece_list[8*r+f] != no_piece) // If there is a piece on this square
                piece = piece_to_char(state.piece_list[8*r+f]); // Set the piece to be printed

            cout << "| " << piece << " ";

        }

        cout << "| " << r+1 << endl;
    }

    // Print the board state flags

    cout << "+---+---+---+---+---+---+---+---+\n  a   b   c   d   e   f   g   h" << endl;
    cout << "Side to move: " << ((state.side_to_move == white) ? "White" : "Black") << endl;
    cout << "Castling rights: " << int(state.castling_rights) << endl;
    cout << "Enpassant square: " << ((state.enpassant_square == no_square) ? "-" : square_to_string(state.enpassant_square)) << endl;
    cout << "Halfmove clock: " << state.halfmove_clock << endl;
    cout << "Fullmove counter: " << state.fullmove_counter << endl;
    cout << "Hash Key: " << state.hash_key << endl;
}

Move Board::generate_move_nopromo(int from_sq, int to_sq) {
    Move move = from_sq | (to_sq << 6);
    int code = 0;
    bool is_pawn = (state.piece_list[from_sq] == P || state.piece_list[from_sq] == p);
    
    if (is_pawn && (to_sq - from_sq == 16 || to_sq - from_sq == -16))
        code = dbpush;

    if (state.piece_list[from_sq] == K && from_sq == e1) {
        if (to_sq == g1) code = kcastle;
        if (to_sq == c1) code = qcastle;
    }

    if (state.piece_list[from_sq] == k && from_sq == e8) {
        if (to_sq == g8) code = kcastle;
        if (to_sq == c8) code = qcastle;
    }

    if (state.piece_list[to_sq] != no_piece)
        code = capture;

    if ((to_sq == state.enpassant_square) && is_pawn)
        code = epcapture;

    return move | (code << 12);
    
}

// Non sliding pieces attacks by calculation

template <bool GEN_CAPTURES>
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
    BB check_mask = ~0;
    BB hor_inbetween = 0, ver_inbetween = 0, dia_inbetween = 0, antdia_inbetween = 0;
    BB occ_ex_king = ((state.bitboards[k] | state.bitboards[K]) & ~opponent_pieces) ^ occ;
    BB opp_any_attacks = 0;
    int king_sq = bitscan_forward((state.bitboards[k] | state.bitboards[K]) & friendly_pieces);
    BB king_super_dia = bishop_moves(king_sq, occ);
    BB king_super_orth = rook_moves(king_sq, occ);
    BB kingBB = mask(king_sq);
    BB opp_attacks = 0;
    BB king_super = 0;

    BB opp_rooks = (state.bitboards[r] | state.bitboards[R] | state.bitboards[q] | state.bitboards[Q]) & opponent_pieces;
    
    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, west);
    opp_any_attacks |= opp_attacks;
    king_super = sliding_attacks(kingBB, occ, east);
    hor_inbetween |= opp_attacks & king_super;

    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, east);
    opp_any_attacks |= opp_attacks;
    king_super = sliding_attacks(kingBB, occ, west);
    hor_inbetween |= opp_attacks & king_super;

    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, nort);
    opp_any_attacks |= opp_attacks;
    king_super = sliding_attacks(kingBB, occ, sout);
    ver_inbetween |= opp_attacks & king_super;

    opp_attacks = sliding_attacks(opp_rooks, occ_ex_king, sout);
    opp_any_attacks |= opp_attacks;
    king_super = sliding_attacks(kingBB, occ, nort);
    ver_inbetween |= opp_attacks & king_super;
    
    BB opp_bishops = (state.bitboards[b] | state.bitboards[B] | state.bitboards[q] | state.bitboards[Q]) & opponent_pieces;
    
    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, noEast);
    opp_any_attacks |= opp_attacks;
    king_super = sliding_attacks(kingBB, occ, soWest);
    dia_inbetween |= opp_attacks & king_super;

    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, soWest);
    opp_any_attacks |= opp_attacks;
    king_super = sliding_attacks(kingBB, occ, noEast);
    dia_inbetween |= opp_attacks & king_super;

    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, soEast);
    opp_any_attacks |= opp_attacks;
    king_super = sliding_attacks(kingBB, occ, noWest);
    antdia_inbetween |= opp_attacks & king_super;

    opp_attacks = sliding_attacks(opp_bishops, occ_ex_king, noWest);
    opp_any_attacks |= opp_attacks;
    king_super = sliding_attacks(kingBB, occ, soEast);
    antdia_inbetween |= opp_attacks & king_super;

    // Non-sliding pieces
    opp_any_attacks |= knight_attacks((state.bitboards[n] | state.bitboards[N]) & opponent_pieces);

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

    while (king_movement) {
        int to_sq = pop_lsb(king_movement);
        state.move_list.add(generate_move_nopromo(king_sq, to_sq));
    }
    
    // If dbl check only king moves are allowed
    if (move_mask == 0) return;
    
    // Pinned knights cannot move
    BB knights = (state.bitboards[n] | state.bitboards[N]) & friendly_pieces & ~all_inbetween;
    while (knights) {
        int from_sq = pop_lsb(knights);
        BB movement = knight_move_table[from_sq] & move_mask;
        while (movement) {
            int to_sq = pop_lsb(movement);
            state.move_list.add(generate_move_nopromo(from_sq, to_sq));
        }
    }

    // Rook and Queen moves
    BB rooks = (state.bitboards[r] | state.bitboards[R] | state.bitboards[q] | state.bitboards[Q]) & friendly_pieces;

    // horizontal
    BB sliders = rooks & ~(all_inbetween ^ hor_inbetween);
    while (sliders) {
        int from_sq = pop_lsb(sliders);

        BB slider_movement = rook_moves(from_sq, occ) & move_mask & hor_fill(from_sq);
        while (slider_movement) {
            int to_sq = pop_lsb(slider_movement);
            state.move_list.add(generate_move_nopromo(from_sq, to_sq));
        }
    }

    // vertical
    sliders = rooks & ~(all_inbetween ^ ver_inbetween);
    while (sliders) {
        int from_sq = pop_lsb(sliders);

        BB slider_movement = rook_moves(from_sq, occ) & move_mask & ver_fill(from_sq);
        while (slider_movement) {
            int to_sq = pop_lsb(slider_movement);
            state.move_list.add(generate_move_nopromo(from_sq, to_sq));
        }
    }

    // Queen and bishop moves
    BB bishops = (state.bitboards[b] | state.bitboards[B] | state.bitboards[q] | state.bitboards[Q]) & friendly_pieces;

    // Diagonal
    sliders = bishops & ~(all_inbetween ^ dia_inbetween);
    while (sliders) {
        int from_sq = pop_lsb(sliders);

        BB slider_movement = bishop_moves(from_sq, occ) & move_mask & dia_fill(from_sq);
        while (slider_movement) {
            int to_sq = pop_lsb(slider_movement);
            state.move_list.add(generate_move_nopromo(from_sq, to_sq));
        }
    }

    // Antidiagonal
    sliders = bishops & ~(all_inbetween ^ antdia_inbetween);
    while (sliders) {
        int from_sq = pop_lsb(sliders);

        BB slider_movement = bishop_moves(from_sq, occ) & move_mask & antdia_fill(from_sq);
        while (slider_movement) {
            int to_sq = pop_lsb(slider_movement);
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
        int from_sq = pop_lsb(pawns);

        // Single push
        int to_sq = from_sq + shifts[state.side_to_move ^ 1];
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
    BB targets = (opponent_pieces & move_mask) | mask(state.enpassant_square);
    pawns = (state.bitboards[p] | state.bitboards[P]) & friendly_pieces;

    BB attacking = pawns & ~(all_inbetween ^ dia_inbetween);
    while (attacking) {
        int from_sq = pop_lsb(attacking);

        BB movement_mask = pawn_attack_table[from_sq][state.side_to_move] & targets & dia_fill(from_sq);
        while (movement_mask) {
            int to_sq = pop_lsb(movement_mask);

            if (to_sq == state.enpassant_square) {
                if (null_if_check) {
                    BB c_occ = occ;
                    pop_bit(c_occ, from_sq);
                    pop_bit(c_occ, (state.side_to_move == white ? to_sq - 8 : to_sq + 8));
                    if (hor_fill(king_sq) & rook_moves(king_sq, c_occ) & opp_rooks) {
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

    attacking = pawns & ~(all_inbetween ^ antdia_inbetween);
    while (attacking) {
        int from_sq = pop_lsb(attacking);

        BB movement_mask = pawn_attack_table[from_sq][state.side_to_move] & targets & antdia_fill(from_sq);
        while (movement_mask) {
            int to_sq = pop_lsb(movement_mask);

            if (to_sq == state.enpassant_square) {
                if (null_if_check) {
                    BB c_occ = occ;
                    pop_bit(c_occ, from_sq);
                    pop_bit(c_occ, (state.side_to_move == white ? to_sq - 8 : to_sq + 8));
                    if (hor_fill(king_sq) & rook_moves(king_sq, c_occ) & opp_rooks) {
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
    if (null_if_check) {
        if (state.side_to_move == white) {
            if (
                // Can castle
                (state.castling_rights & wking_side)
                
                // Squares king travel aren't attacked
                && !(get_bit(opp_any_attacks, f1) || get_bit(opp_any_attacks, g1))
                
                // There are no pieces between king & rook
                
                && !(get_bit(occ, f1) || get_bit(occ, g1))
                
            )
            state.move_list.add(generate_move_nopromo(e1, g1));

            if (
                // Can castle
                (state.castling_rights & wqueen_side)

                // Squares king travel aren't attacked
                && !(get_bit(opp_any_attacks, d1) || get_bit(opp_any_attacks, c1))

                // There are no pieces between king & rook

                && !(get_bit(occ, d1) || get_bit(occ, c1) || get_bit(occ, b1))
            
            )
            state.move_list.add(generate_move_nopromo(e1, c1));

        } else {
            if (
                // Can castle
                (state.castling_rights & bking_side)

                // Squares king travel aren't attacked
                && !(get_bit(opp_any_attacks, f8) || get_bit(opp_any_attacks, g8))

                // There are no pieces between king & rook

                && !(get_bit(occ, f8) || get_bit(occ, g8))
            
            )
            state.move_list.add(generate_move_nopromo(e8, g8));

            if (
                // Can castle
                (state.castling_rights & bqueen_side)

                // Squares king travel aren't attacked
                && !(get_bit(opp_any_attacks, d8) || get_bit(opp_any_attacks, c8))

                // There are no pieces between king & rook

                && !(get_bit(occ, d8) || get_bit(occ, c8) || get_bit(occ, b8))
            
            )
            state.move_list.add(generate_move_nopromo(e8, c8));
        }
    }

}

template void Board::generate_moves<CAPTURES>();
template void Board::generate_moves<ALLMOVES>();

void Board::make_move(Move move) {
    prev_states.push_back(state);
    if (move == nullmove) {
        if (state.side_to_move == black) state.fullmove_counter++;
        state.side_to_move = state.side_to_move == black ? white : black;
        state.enpassant_square = no_square;
        state.halfmove_clock = 0;
        state.hash_key = zobrist::gen_pos_key(state);
        return;
    }

    Squares from_sq = Squares(move & 0b111111), to_sq = Squares((move >> 6) & 0b111111);
    Pieces piece = state.piece_list.at(from_sq);

    int move_code = move >> 12;

    pop_bit(state.bitboards[piece], from_sq);
    state.piece_list[from_sq] = no_piece;
    set_bit(state.bitboards[piece], to_sq);
    
    if (move_code == capture || move_code >= c_npromo) {
        Pieces c_piece = state.piece_list.at(to_sq);
        pop_bit(state.bitboards[c_piece], to_sq); 
    }
    
    state.piece_list[to_sq] = piece;
    
    if (move_code == epcapture) {
        if (state.side_to_move == white) {
            pop_bit(state.bitboards[p], state.enpassant_square - 8);
            state.piece_list[state.enpassant_square - 8] = no_piece;
        } else {
            pop_bit(state.bitboards[P], state.enpassant_square + 8);
            state.piece_list[state.enpassant_square + 8] = no_piece;
        }
    }

    if (move_code >= npromo)
        pop_bit(state.bitboards[piece], to_sq);

    if (move_code == npromo || move_code == c_npromo) {
        if (state.side_to_move == white) {
            state.piece_list[to_sq] = N;
            set_bit(state.bitboards[N], to_sq);
        } else {
            state.piece_list[to_sq] = n;
            set_bit(state.bitboards[n], to_sq);
        }
    }

    if (move_code == bpromo || move_code == c_bpromo) {
        if (state.side_to_move == white) {
            state.piece_list[to_sq] = B;
            set_bit(state.bitboards[B], to_sq);
        } else {
            state.piece_list[to_sq] = b;
            set_bit(state.bitboards[b], to_sq);
        }
    }
    
    if (move_code == rpromo || move_code == c_rpromo) {
        if (state.side_to_move == white) {
            state.piece_list[to_sq] = R;
            set_bit(state.bitboards[R], to_sq);
        } else {
            state.piece_list[to_sq] = r;
            set_bit(state.bitboards[r], to_sq);
        }
    }

    if (move_code == qpromo || move_code == c_qpromo) {
        if (state.side_to_move == white) {
            state.piece_list[to_sq] = Q;
            set_bit(state.bitboards[Q], to_sq);
        } else {
            state.piece_list[to_sq] = q;
            set_bit(state.bitboards[q], to_sq);
        }
    }

    if (move_code == kcastle) {
        if (state.side_to_move == white) {
            pop_bit(state.bitboards[R], h1);
            state.piece_list[h1] = no_piece;
            set_bit(state.bitboards[R], f1);
            state.piece_list[f1] = R;
        } else {
            pop_bit(state.bitboards[r], h8);
            state.piece_list[h8] = no_piece;
            set_bit(state.bitboards[r], f8);
            state.piece_list[f8] = r;
        }
    }

    if (move_code == qcastle) {
        if (state.side_to_move == white) {
            pop_bit(state.bitboards[R], a1);
            state.piece_list[a1] = no_piece;
            set_bit(state.bitboards[R], d1);
            state.piece_list[d1] = R;
        } else {
            pop_bit(state.bitboards[r], a8);
            state.piece_list[a8] = no_piece;
            set_bit(state.bitboards[r], d8);
            state.piece_list[d8] = r;
        }
    }

    array<int, 64> castle_encoder = {
        13, 15, 15, 15, 12, 15, 15, 14,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
         7, 15, 15, 15,  3, 15, 15, 11
    };

    state.castling_rights &= castle_encoder[from_sq];
    state.castling_rights &= castle_encoder[to_sq];

    if (move_code == dbpush) {
        if (state.side_to_move == white)
            state.enpassant_square = Squares(to_sq - 8);
        else 
            state.enpassant_square = Squares(to_sq + 8);
    } else
        state.enpassant_square = no_square;

    if (state.side_to_move == black) state.fullmove_counter++;
    state.side_to_move = state.side_to_move == black ? white : black;
    if (move_code == capture || piece == p || piece == P)
        state.halfmove_clock = 0;
    else
        state.halfmove_clock++;

    state.bitboards[12] = state.bitboards[p] | state.bitboards[n] | state.bitboards[b] |
                          state.bitboards[r] | state.bitboards[q] | state.bitboards[k];
    state.bitboards[13] = state.bitboards[P] | state.bitboards[N] | state.bitboards[B] |
                          state.bitboards[R] | state.bitboards[Q] | state.bitboards[K];
    state.bitboards[14] = state.bitboards[12] | state.bitboards[13];

    state.hash_key = zobrist::gen_pos_key(state);
}

void Board::unmake_last_move() {
    if (prev_states.size() > 1) {
        state = prev_states.back();
        prev_states.pop_back();
    }
}

// bool Board::is_over() {
//     if (move_list.is_empty())
//         return true;
//     return false;
// }