#include "../include/utils.h"
#include "../include/board.h"
#include "../include/move.h"
#include "../include/bitboard_utils.h"

/*
Moves are encoded in a 16-bit integer:
6 bits for the from square
6 bits for the to square
4 bits for a code

code | Promo | capt | special 1 | sp 2 | type
0    | 0     | 0    | 0         | 0    | quiet
1    | 0     | 0    | 0         | 1    | dbp pawn
2    | 0     | 0    | 1         | 0    | king castle
3    | 0     | 0    | 1         | 1    | q castle
4    | 0     | 1    | 0         | 0    | capture
5    | 0     | 1    | 0         | 1    | ep capture
8    | 1     | 0    | 0         | 0    | knight promo
9    | 1     | 0    | 0         | 1    | bishop promo
10   | 1     | 0    | 1         | 0    | rook promo
11   | 1     | 0    | 1         | 1    | queen promo
12   | 1     | 1    | 0         | 0    | knight promo capt
13   | 1     | 1    | 0         | 1    | bishop promo capt
14   | 1     | 1    | 1         | 0    | rook promo capt
15   | 1     | 1    | 1         | 1    | queen promo capt

*/

/* Kogge-stone fill algorithms for sliding pieces attacks */

inline BB south_attacks(BB rooks, BB empty) {
    rooks |= empty & (rooks >> 8);
    empty &= empty >> 8;
    rooks |= empty & (rooks >> 16);
    empty &= empty >> 16;
    rooks |= empty & (rooks >> 32);
    return rooks >> 8;
}

inline BB north_attacks(BB rooks, BB empty) {
    rooks |= empty & (rooks << 8);
    empty &= empty << 8;
    rooks |= empty & (rooks << 16);
    empty &= empty << 16;
    rooks |= empty & (rooks << 32);
    return rooks << 8;
}

inline BB east_attacks(BB rooks, BB empty) {
    empty &= nAFILE;
    rooks |= empty & (rooks << 1);
    empty &= empty << 1;
    rooks |= empty & (rooks << 2);
    empty &= empty << 2;
    rooks |= empty & (rooks << 4);
    return (rooks << 1) & nAFILE;
}

inline BB west_attacks(BB rooks, BB empty) {
    empty &= nHFILE;
    rooks |= empty & (rooks >> 1);
    empty &= empty >> 1;
    rooks |= empty & (rooks >> 2);
    empty &= empty >> 2;
    rooks |= empty & (rooks >> 4);
    return (rooks >> 1) & nHFILE;
}

inline BB no_east_attacks(BB bishops, BB empty) {
    empty &= nAFILE;
    bishops |= empty & (bishops <<  9);
    empty &=   empty << 9;
    bishops |= empty & (bishops << 18);
    empty &=   empty << 18;
    bishops |= empty & (bishops << 36);
    return (bishops << 9) & nAFILE;
}

inline BB no_west_attacks(BB bishops, BB empty) {
    empty &= nHFILE;
    bishops |= empty & (bishops << 7);
    empty &=   empty << 7;
    bishops |= empty & (bishops << 14);
    empty &=   empty << 14;
    bishops |= empty & (bishops << 28);
    return (bishops << 7) & nHFILE;
}

inline BB so_east_attacks(BB bishops, BB empty) {
    empty &= nAFILE;
    bishops |= empty & (bishops >> 7);
    empty &=   empty >> 7;
    bishops |= empty & (bishops >> 14);
    empty &=   empty >> 14;
    bishops |= empty & (bishops >> 28);
    return (bishops >> 7) & nAFILE;
}

inline BB so_west_attacks(BB bishops, BB empty) {
    empty &= nHFILE;
    bishops |= empty & (bishops >> 9);
    empty &=   empty >> 9;
    bishops |= empty & (bishops >> 18);
    empty &=   empty >> 18;
    bishops |= empty & (bishops >> 36);
    return (bishops >> 9) & nHFILE;
}

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

void generate_moves(Board &board, std::vector<int> &move_list) {
    
    /* Dir golem algorithm */

    BB opp_pawns = board.bitboards[p];
    BB opp_knights = board.bitboards[n];
    BB opp_bishops = board.bitboards[b];
    BB opp_rooks = board.bitboards[r];
    BB opp_queens = board.bitboards[q];
    BB opp_king = board.bitboards[k];
    BB king_bb = board.bitboards[K];

    BB friendly_pieces = board.bitboards[P] | board.bitboards[N] |
                        board.bitboards[B] | board.bitboards[R] |
                        board.bitboards[Q] | board.bitboards[K];

    if (board.side_to_move == black) {
        opp_pawns = board.bitboards[P];
        opp_knights = board.bitboards[N];
        opp_bishops = board.bitboards[B];
        opp_rooks = board.bitboards[R];
        opp_queens = board.bitboards[Q];
        opp_king = board.bitboards[K];
        king_bb = board.bitboards[k];

        friendly_pieces = board.bitboards[p] | board.bitboards[n] |
                         board.bitboards[b] | board.bitboards[r] |
                         board.bitboards[q] | board.bitboards[k];
    }

    BB hor_in_between, ver_in_between, dia_in_between, ant_in_between;
    BB k_super_attacks_orth, k_super_attacks_dia, opp_any_attacks;

    BB occ = board.bitboards[allpieces];

    /* Kogge-stone fill for black sliding attacks */

    BB _opp_attacks;
    BB _k_super_attacks;

    // Opp rooks and queens south atttacks
    _opp_attacks = south_attacks(opp_rooks | opp_queens, ~occ ^ king_bb);
    _k_super_attacks = north_attacks(king_bb, ~occ);
    opp_any_attacks = _opp_attacks;
    k_super_attacks_orth = _k_super_attacks;
    ver_in_between = _opp_attacks & _k_super_attacks;

    // Opp rooks and queens north atttacks
    _opp_attacks = north_attacks(opp_rooks | opp_queens, ~occ ^ king_bb);
    _k_super_attacks = south_attacks(king_bb, ~occ);
    opp_any_attacks |= _opp_attacks;
    k_super_attacks_orth |= _k_super_attacks;
    ver_in_between |= _opp_attacks & _k_super_attacks;

    // Opp rooks and queens east atttacks
    _opp_attacks = east_attacks(opp_rooks | opp_queens, ~occ ^ king_bb);
    _k_super_attacks = west_attacks(king_bb, ~occ);
    opp_any_attacks |= _opp_attacks;
    k_super_attacks_orth |= _k_super_attacks;
    hor_in_between = _opp_attacks & _k_super_attacks;

    // Opp rooks and queens west atttacks
    _opp_attacks = west_attacks(opp_rooks | opp_queens, ~occ ^ king_bb);
    _k_super_attacks = east_attacks(king_bb, ~occ);
    opp_any_attacks |= _opp_attacks;
    k_super_attacks_orth |= _k_super_attacks;
    hor_in_between |= _opp_attacks & _k_super_attacks;



    // Opp bishops and queens south east attacks
    _opp_attacks = so_east_attacks(opp_bishops | opp_queens, ~occ ^ king_bb);
    _k_super_attacks = no_west_attacks(king_bb, ~occ);
    opp_any_attacks |= _opp_attacks;
    k_super_attacks_dia |= _k_super_attacks;
    dia_in_between = _opp_attacks & _k_super_attacks;

    // Opp bishops and queens south west attacks
    _opp_attacks = so_west_attacks(opp_bishops | opp_queens, ~occ ^ king_bb);
    _k_super_attacks = no_east_attacks(king_bb, ~occ);
    opp_any_attacks |= _opp_attacks;
    k_super_attacks_dia |= _k_super_attacks;
    dia_in_between |= _opp_attacks & _k_super_attacks;

    // Opp bishops and queens north east attacks
    _opp_attacks = no_west_attacks(opp_bishops | opp_queens, ~occ ^ king_bb);
    _k_super_attacks = so_east_attacks(king_bb, ~occ);
    opp_any_attacks |= _opp_attacks;
    k_super_attacks_dia |= _k_super_attacks;
    dia_in_between |= _opp_attacks & _k_super_attacks;

    // Opp bishops and queens north west attacks
    _opp_attacks = no_east_attacks(opp_bishops | opp_queens, ~occ ^ king_bb);
    _k_super_attacks = so_west_attacks(king_bb, ~occ);
    opp_any_attacks |= _opp_attacks;
    k_super_attacks_dia |= _k_super_attacks;
    dia_in_between |= _opp_attacks & _k_super_attacks;

    /* Non Sliding Pieces */

    // Opp knight attacks

    opp_any_attacks |= knight_attacks(opp_knights);

    // Opp pawn attacks

    if (board.side_to_move == white) {
        opp_any_attacks |= bpawn_attacks(opp_pawns);
    } else
        opp_any_attacks |= wpawn_attacks(opp_pawns);

    // Opp king attacks

    opp_any_attacks |= king_attacks(opp_king);

    /* Generating Moves */
    
    BB all_in_between = hor_in_between | ver_in_between | dia_in_between;
    
    /* Handle Check */

    BB _blocks = all_in_between & ~occ;
    BB _check_From = (k_super_attacks_orth & (opp_rooks | opp_queens))
                    | (k_super_attacks_dia & (opp_bishops | opp_queens))
                    | (knight_attacks(opp_knights) & opp_knights)
                    | ((board.side_to_move == white) ? bpawn_attacks(opp_pawns) : wpawn_attacks(opp_pawns));

    int64_t _null_if_check = (static_cast<int64_t>(opp_any_attacks & king_bb) - 1) >> 63;
    int64_t _null_if_DBl_check = (static_cast<int64_t>(_check_From & (_check_From-1)) - 1) >> 63;

    BB _check_to = _check_From | _blocks | _null_if_check;
    BB target_mask = ~friendly_pieces & _check_to & _null_if_DBl_check;
    
    /* Sliding Pieces */



    /* Non Sliding Pieces */


}