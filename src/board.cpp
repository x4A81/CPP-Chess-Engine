#include "../include/board.h"
#include "../include/utils.h"
#include "../include/bitboard_utils.h"
#include <iostream>
#include <sstream>

using namespace bitboard_utils;
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
    cout << "Enpassant square: " << ((state.enpassant_square == no_square) ? "-" : sqauare_to_string(state.enpassant_square)) << endl;
    cout << "Halfmove clock: " << state.halfmove_clock << endl;
    cout << "Fullmove counter: " << state.fullmove_counter << endl;
    cout << "Hash Key: " << state.hash_key << endl;
}

/* Move gen code */

// Non sliding pieces attacks by calculation
inline BB knight_attacks(BB knights) {
    BB l1 = (knights >> 1) & 0x7f7f7f7f7f7f7f7f;
    BB l2 = (knights >> 2) & 0x3f3f3f3f3f3f3f3f;
    BB r1 = (knights << 1) & 0xfefefefefefefefe;
    BB r2 = (knights << 2) & 0xfcfcfcfcfcfcfcfc;
    BB h1 = l1 | r1;
    BB h2 = l2 | r2;
    return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
}

inline BB wpawn_attacks(BB pawns) {
    return ((pawns << 9) & nAFILE) | (pawns << 7) & nHFILE;
}

inline BB bpawn_attacks(BB pawns) {
    return ((pawns >> 9) & nHFILE) | ((pawns >> 7) & nAFILE);
}

inline BB king_attacks(BB king) {
    BB attacks = ((king >> 1) & nHFILE) | ((king << 1) & nAFILE);
    king |= attacks;
    attacks |= (king >> 8) | (king << 8);
    return attacks;
}

inline BB knight_noNoEa(BB bb) { return (bb << 17) & nAFILE; }
inline BB knight_noEaEa(BB bb) { return (bb << 10) & nABFILE; }
inline BB knight_soEaEa(BB bb) { return (bb >>  6) & nABFILE; }
inline BB knight_soSoEa(BB bb) { return (bb >> 15) & nAFILE; }
inline BB knight_noNoWe(BB bb) { return (bb << 15) & nHFILE; }
inline BB knight_noWeWe(BB bb) { return (bb <<  6) & nGHFILE; }
inline BB knight_soWeWe(BB bb) { return (bb >> 10) & nGHFILE; }
inline BB knight_soSoWe(BB bb) { return (bb >> 17) & nHFILE; }

array<BB, 16> Board::generate_move_targets() {

    array<BB, 16> move_targets;
    move_targets.fill(BB(0));

    
    /* Dir golem algorithm */
    
    // The Opps

    BB opp_pawns = state.bitboards[p];
    BB opp_knights = state.bitboards[n];
    BB opp_bishops = state.bitboards[b];
    BB opp_rooks = state.bitboards[r];
    BB opp_queens = state.bitboards[q];
    BB opp_king = state.bitboards[k];

    BB opp_pieces = opp_pawns | opp_knights |
                        opp_bishops | opp_rooks |
                        opp_queens | opp_king;

    BB king_bb = state.bitboards[K];

    BB friendly_pieces = state.bitboards[P] | state.bitboards[N] |
                        state.bitboards[B] | state.bitboards[R] |
                        state.bitboards[Q] | state.bitboards[K];

    BB friendly_pawns = state.bitboards[P];
    BB friend_knights = state.bitboards[N];
    BB friend_rooks = state.bitboards[R];
    BB friend_bishops = state.bitboards[B];
    BB friend_queens = state.bitboards[Q];

    if (state.side_to_move == black) {
        opp_pawns = state.bitboards[P];
        opp_knights = state.bitboards[N];
        opp_bishops = state.bitboards[B];
        opp_rooks = state.bitboards[R];
        opp_queens = state.bitboards[Q];
        opp_king = state.bitboards[K];
        king_bb = state.bitboards[k];

        friendly_pieces = state.bitboards[p] | state.bitboards[n] |
                         state.bitboards[b] | state.bitboards[r] |
                         state.bitboards[q] | state.bitboards[k];
                         
        friendly_pawns = state.bitboards[p];
        friend_knights = state.bitboards[n];
        friend_rooks = state.bitboards[r];
        friend_bishops = state.bitboards[b];
        friend_queens = state.bitboards[q];
    }

    BB hor_in_between = 0, ver_in_between = 0, dia_in_between = 0, ant_in_between = 0;
    BB k_super_attacks_orth = 0, k_super_attacks_dia = 0, opp_any_attacks = 0;

    BB occ = state.bitboards[allpieces];

    /* Kogge-stone fill for black sliding attacks */

    BB opp_attacks = 0;
    BB k_super_attacks = 0;
    BB occ_ex_king = ~occ ^ king_bb;

    // If there aren't any pieces to process, skip
    BB pieces = opp_rooks | opp_queens;
    if (pieces) {
        // Opp rooks and queens south atttacks
        opp_attacks = sliding_attacks(pieces, occ_ex_king, sout);
        k_super_attacks = sliding_attacks(king_bb, occ, nort);
        opp_any_attacks |= opp_attacks;
        k_super_attacks_orth |= k_super_attacks;
        ver_in_between |= opp_attacks & k_super_attacks;
    
        // Opp rooks and queens north atttacks
        opp_attacks = sliding_attacks(pieces, occ_ex_king, nort);
        k_super_attacks = sliding_attacks(king_bb, occ, sout);
        opp_any_attacks |= opp_attacks;
        k_super_attacks_orth |= k_super_attacks;
        ver_in_between |= opp_attacks & k_super_attacks;
    
        // Opp rooks and queens east atttacks
        opp_attacks = sliding_attacks(pieces, occ_ex_king, east);
        k_super_attacks = sliding_attacks(king_bb, occ, west);
        opp_any_attacks |= opp_attacks;
        k_super_attacks_orth |= k_super_attacks;
        hor_in_between |= opp_attacks & k_super_attacks;
    
        // Opp rooks and queens west atttacks
        opp_attacks = sliding_attacks(pieces, occ_ex_king, west);
        k_super_attacks = sliding_attacks(king_bb, occ, east);
        opp_any_attacks |= opp_attacks;
        k_super_attacks_orth |= k_super_attacks;
        hor_in_between |= opp_attacks & k_super_attacks;
    }

    pieces = opp_bishops | opp_queens;
    if (pieces) {

        // Opp bishops and queens south west attacks
        opp_attacks = sliding_attacks(pieces, occ_ex_king, soWest);
        k_super_attacks = sliding_attacks(king_bb, occ, noEast);
        opp_any_attacks |= opp_attacks;
        k_super_attacks_dia |= k_super_attacks;
        dia_in_between |= opp_attacks & k_super_attacks;
    
        // Opp bishops and queens north east attacks
        opp_attacks = sliding_attacks(pieces, occ_ex_king, noEast);
        k_super_attacks = sliding_attacks(king_bb, occ, noWest);
        opp_any_attacks |= opp_attacks;
        k_super_attacks_dia |= k_super_attacks;
        dia_in_between |= opp_attacks & k_super_attacks;
    
        // Opp bishops and queens north west attacks
        opp_attacks = sliding_attacks(pieces, occ_ex_king, noWest);
        k_super_attacks = sliding_attacks(king_bb, occ, soEast);
        opp_any_attacks |= opp_attacks;
        k_super_attacks_dia |= k_super_attacks;
        ant_in_between |= opp_attacks & k_super_attacks;
    
        // Opp bishops and queens south east attacks
        opp_attacks = sliding_attacks(pieces, occ_ex_king, soEast);
        k_super_attacks = sliding_attacks(king_bb, occ, noWest);
        opp_any_attacks |= opp_attacks;
        k_super_attacks_dia |= k_super_attacks;
        ant_in_between |= opp_attacks & k_super_attacks;
    }

    /* Non Sliding Pieces */

    // Opp knight attacks

    opp_any_attacks |= knight_attacks(opp_knights);

    // Opp pawn attacks

    if (state.side_to_move == white) {
        opp_any_attacks |= bpawn_attacks(opp_pawns);
    } else
        opp_any_attacks |= wpawn_attacks(opp_pawns);

    // Opp king attacks

    opp_any_attacks |= king_attacks(opp_king);

    /* Generating Moves */
    
    BB all_in_between = hor_in_between | ver_in_between | dia_in_between | ant_in_between;

    /* Handle Check */

    BB blocks = all_in_between & ~occ;
    BB check_from = (k_super_attacks_orth & (opp_rooks | opp_queens))
                    | (k_super_attacks_dia & (opp_bishops | opp_queens))
                    | (knight_attacks(king_bb) & opp_knights)
                    | ((state.side_to_move == white) ? (wpawn_attacks(king_bb) & opp_pawns) : (bpawn_attacks(king_bb) & opp_pawns));

    int64_t null_if_check = (int64_t(opp_any_attacks & king_bb) - 1) >> 63;
    int64_t null_if_DBl_check = (int64_t(check_from & (check_from-1)) - 1) >> 63;

    BB check_to = check_from | blocks | null_if_check;
    BB target_mask = ~friendly_pieces & check_to & null_if_DBl_check;
    
    BB sliders = 0;

    /* Sliding Pieces */
    pieces = friend_rooks | friend_queens;
    if (pieces) {
        sliders = (friend_rooks | friend_queens) & ~(all_in_between ^ hor_in_between);
        move_targets[east] |= sliding_attacks(sliders, occ, east) & target_mask;
        move_targets[west] |= sliding_attacks(sliders, occ, west) & target_mask;
        
        sliders = (friend_rooks | friend_queens) & ~(all_in_between ^ ver_in_between);
        move_targets[nort] |= sliding_attacks(sliders, occ, nort) & target_mask;
        move_targets[sout] |= sliding_attacks(sliders, occ, sout) & target_mask;
    }

    pieces = friend_bishops | friend_queens;
    if (pieces) {
        sliders = (friend_bishops | friend_queens) & ~(all_in_between ^ dia_in_between);
        move_targets[soWest] |= sliding_attacks(sliders, occ, soWest) & target_mask;
        move_targets[noEast] |= sliding_attacks(sliders, occ, noEast) & target_mask;
    
        sliders = (friend_bishops | friend_queens) & ~(all_in_between ^ ant_in_between);
        move_targets[soEast] |= sliding_attacks(sliders, occ, soEast) & target_mask;
        move_targets[noWest] |= sliding_attacks(sliders, occ, noWest) & target_mask;
    }

    /* Non Sliding Pieces */

    // Knights
    BB knights = friend_knights & ~all_in_between;
    if (knights) {
        move_targets[noNoEa] = knight_noNoEa(knights) & target_mask;
        move_targets[noEaEa] = knight_noEaEa(knights) & target_mask;
        move_targets[soEaEa] = knight_soEaEa(knights) & target_mask;
        move_targets[soSoEa] = knight_soSoEa(knights) & target_mask;
        move_targets[noNoWe] = knight_noNoWe(knights) & target_mask;
        move_targets[noWeWe] = knight_noWeWe(knights) & target_mask;   
        move_targets[soWeWe] = knight_soWeWe(knights) & target_mask;
        move_targets[soSoWe] = knight_soSoWe(knights) & target_mask;
    }

    // Pawn Captures including en passant

    BB hor_between = 0;
    BB targets = (opp_pieces & target_mask);
    if (state.side_to_move == white) {
        
        if (state.enpassant_square != no_square && null_if_check) {
            BB c_occ = occ & ~mask(state.enpassant_square + 8);
            set_bit(c_occ, state.enpassant_square);
            hor_between = sliding_attacks(king_bb, occ, east) | sliding_attacks(king_bb, occ, west);
            if (!(hor_between & friendly_pawns))
            set_bit(targets, state.enpassant_square);
        }
        
    } else {
        if (state.enpassant_square != no_square && null_if_check) {
            BB c_occ = occ & ~mask(state.enpassant_square - 8);
            set_bit(c_occ, state.enpassant_square);
            hor_between = sliding_attacks(king_bb, occ, east) | sliding_attacks(king_bb, occ, west);
            if (!(hor_between & friendly_pawns))
            set_bit(targets, state.enpassant_square);
        }
    }

    BB pawns = friendly_pawns & ~(all_in_between ^ dia_in_between);
    if (pawns) {
        if (state.side_to_move == white)
            move_targets[noEast] |= shift_one(pawns, noEast) & targets;
        else
            move_targets[soWest] |= shift_one(pawns, soWest) & targets;
    }

    pawns = friendly_pawns & ~(all_in_between ^ ant_in_between);
    if (pawns) {
        if (state.side_to_move == white)
            move_targets[noWest] |= shift_one(pawns, noWest) & targets;
        else
            move_targets[soEast] |= shift_one(pawns, noEast) & targets;
    }
        
    // Pawn Pushes
    pawns = friendly_pawns & ~(all_in_between ^ ver_in_between);
    BB pawn_pushes = 0;
    BB dbl_rank = 0;
    
    if (pawns) {
        if (state.side_to_move == white) {
            pawn_pushes = (pawns << 8) & ~occ;
            move_targets[nort] |= pawn_pushes & target_mask;
            dbl_rank= 0x00000000FF000000ULL;
            move_targets[nort] |= (pawn_pushes << 8) & ~occ & target_mask & dbl_rank;
            
        } else {
            pawn_pushes |= (pawns >> 8) & ~occ;
            move_targets[sout] |= pawn_pushes & target_mask;
            dbl_rank = 0x000000FF00000000ULL;
            move_targets[sout] |= (pawn_pushes >> 8) & ~occ & target_mask & dbl_rank;
        }
    }

    // King Moves
    target_mask = ~(friendly_pieces | opp_any_attacks);
    move_targets[west] |= ((king_bb << 1) & nHFILE) & target_mask;
    move_targets[east] |= ((king_bb >> 1) & nAFILE) & target_mask;
    move_targets[nort] |= ((king_bb << 8) & target_mask);
    move_targets[sout] |= ((king_bb >> 8) & target_mask);
    move_targets[noEast] |= ((king_bb << 9) & nAFILE) & target_mask;
    move_targets[noWest] |= ((king_bb << 7) & nHFILE) & target_mask;
    move_targets[soEast] |= ((king_bb >> 7) & nAFILE) & target_mask;
    move_targets[soWest] |= ((king_bb >> 9) & nHFILE) & target_mask;

    // Castling
    if (null_if_check) {
        if (state.side_to_move == white) {
            if (state.castling_rights & wking_side) {
                // Check if blocked or attacked
                if (state.piece_list[f1] == no_piece && state.piece_list[g1] == no_piece
                && ~(opp_any_attacks & mask(f1)) && ~(opp_any_attacks & mask(g1)))

                    move_targets[east] |= mask(g1);

            } else if (state.castling_rights & wqueen_side) {
                // Check if blocked or attacked
                if (state.piece_list[d1] == no_piece && state.piece_list[c1] == no_piece && state.piece_list[b1] == no_piece
                && ~(opp_any_attacks & mask(d1)) && ~(opp_any_attacks & mask(c1)))
                
                    move_targets[west] |= mask(c1);
            }
        } else {
            if (state.castling_rights & bking_side) {
                // Check if blocked or attacked
                if (state.piece_list[f8] == no_piece && state.piece_list[g8] == no_piece
                && ~(opp_any_attacks & mask(f8)) && ~(opp_any_attacks & mask(g8)))

                    move_targets[east] |= mask(g8);

            } else if (state.castling_rights & bqueen_side) {
                // Check if blocked or attacked
                if (state.piece_list[d8] == no_piece && state.piece_list[c8] == no_piece && state.piece_list[b8] == no_piece
                && ~(opp_any_attacks & mask(d8)) && ~(opp_any_attacks & mask(c8)))

                    move_targets[west] |= mask(c8);
            }
        }
    }

    return move_targets;
}

Move Board::generate_move_nopromo(Squares from_sq, Squares to_sq) {
    Move move = from_sq | (to_sq << 6);
    int code = 0;
    bool is_pawn = (state.piece_list[from_sq] == P || state.piece_list[from_sq] == p);
    
    if (is_pawn && (to_sq - from_sq == 16 || to_sq - from_sq == -16))
        code = 1;

    if (state.piece_list[from_sq] == K || state.piece_list[from_sq] == k) {
        if (to_sq == g1 || to_sq == g8)
            code = 2;
        if (to_sq == c1 || to_sq == c8)
            code = 3;
    }

    if (state.piece_list[to_sq] != no_piece)
        code = 4;

    if (to_sq == state.enpassant_square && is_pawn)
        code = 5;

    if ((state.piece_list[from_sq] == P && to_sq > 55) || (state.piece_list[from_sq] == p && to_sq < 8))
        code |= 8;

    return move | (code << 12);
    
}

template <bool GEN_CAPTURES>
void Board::generate_moves() {
    state.move_list.clear();
    array<BB, 16> move_targets = generate_move_targets();
    for (int dir = nort; dir <= soWest; dir++) {
        if constexpr (GEN_CAPTURES)
            move_targets[dir] &= state.bitboards[allpieces] | mask(state.enpassant_square);
        
        while (move_targets[dir]) {
            int target = bitscan_forward(move_targets[dir]);
            pop_bit(move_targets[dir], target);

            int from_sq = target - shifts[dir];
            while (state.piece_list.at(from_sq) == no_piece) {
                from_sq -= shifts[dir];
            }

            if constexpr (GEN_CAPTURES) {
                if (target == state.enpassant_square && !(state.piece_list[from_sq] == p || state.piece_list[from_sq] == P))
                    continue;
            }

            Move move = generate_move_nopromo(Squares(from_sq), Squares(target));
            if (get_bit(move, 16)) {
                for (int i = 8; i <= 11; i++) {
                    Move cmove = move | (i << 12);
                    state.move_list.add(cmove);
                }
            } else
                state.move_list.add(move);
        }
    }

    for (int dir = noNoEa; dir <= soSoWe; dir++) {
        if constexpr (GEN_CAPTURES)
            move_targets[dir] &= state.bitboards[allpieces];
        
        while (move_targets[dir]) {
            int target = bitscan_forward(move_targets[dir]);
            pop_bit(move_targets[dir], target);
            int from_sq = target - shifts[dir];
            Move move = generate_move_nopromo(Squares(from_sq), Squares(target));
            state.move_list.add(move);
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

    if (piece > 12) {
        print_piece_list(state.piece_list);
    }

    int move_code = move >> 12;

    pop_bit(state.bitboards[piece], from_sq);
    state.piece_list[from_sq] = no_piece;
    set_bit(state.bitboards[piece], to_sq);
    
    if (move_code == capture || move_code >= npromo) {
        Pieces c_piece = state.piece_list.at(to_sq);
        pop_bit(state.bitboards[c_piece], to_sq); 
    }
    
    state.piece_list[to_sq] = piece;
    
    if (move_code == epcapture) {
        if (state.side_to_move == white) {
            pop_bit(state.bitboards[p], state.enpassant_square - 8);
            state.piece_list[state.enpassant_square - 8] = no_piece;
        } else {
            pop_bit(state.bitboards[p], state.enpassant_square + 8);
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