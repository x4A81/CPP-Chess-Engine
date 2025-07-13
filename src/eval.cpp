#include "../include/eval.hpp"

Score Board::eval_pawns() {
    Score score = 0;

    BB wpawns = state.bitboards[P];
    BB bpawns = state.bitboards[p];
    BB copy_bb;

    // Material.
    score += material[p] * pop_count(wpawns);  
    score -= material[p] * pop_count(bpawns);          
    
    // PSQT.
    copy_bb = wpawns;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score += pawn_psqt[flip_rank(sq)];
    }

    copy_bb = bpawns;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score -= pawn_psqt[sq];
    }

    // Doubled.
    BB wpawns_infront_behind = wpawns & wrear_span(wpawns);
    BB bpawns_infront_behind = bpawns & brear_span(bpawns);

    int num_doubled = pop_count(wpawns_infront_behind) / 2;
    score += DBL_PAWNS_PEN * num_doubled;

    num_doubled = pop_count(bpawns_infront_behind) / 2;
    score -= DBL_PAWNS_PEN * num_doubled;

    // Triples.
    wpawns_infront_behind &= wpawns & wfront_span(wpawns);
    bpawns_infront_behind &= bpawns & bfront_span(bpawns);

    BB file_with_triple = file_fill(wpawns_infront_behind);

    int num_tripled = pop_count(file_with_triple) / 3;
    score += TRI_PAWNS_PEN * num_tripled;

    file_with_triple = file_fill(bpawns_infront_behind);
    num_tripled = pop_count(file_with_triple) / 3;
    score -= TRI_PAWNS_PEN * num_tripled;

    // Passed.
    int num_passed = pop_count(wpassed_pawns(wpawns, bpawns));
    score += num_passed * PASS_PAWNS_BONUS;

    num_passed = pop_count(bpassed_pawns(bpawns, wpawns));
    score -= num_passed * PASS_PAWNS_BONUS;

    // Isolated.
    int num_isolated = pop_count(isolanis(wpawns));
    score += ISO_PAWNS_PEN * num_isolated;

    num_isolated = pop_count(isolanis(bpawns));
    score -= ISO_PAWNS_PEN * num_isolated;

    // Half isolated.
    int num_half_isolated = pop_count(half_isolanis(wpawns));
    score += HALF_ISO_PAWNS_PEN * num_half_isolated;

    num_half_isolated = pop_count(half_isolanis(bpawns));
    score -= HALF_ISO_PAWNS_PEN * num_half_isolated;

    return score;
}

Score Board::eval_knights() {
    Score score = 0;

    BB wknights = state.bitboards[N];
    BB bknights = state.bitboards[n];
    BB copy_bb;

    score += material[n] * pop_count(wknights);
    score -= material[n] * pop_count(bknights);

    copy_bb = wknights;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score += knight_psqt[flip_rank(sq)];
    }
    
    copy_bb = bknights;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score -= knight_psqt[sq];
    }

    return score;
}

Score Board::eval_bishops() {
    Score score = 0;

    BB wbishops = state.bitboards[B];
    BB bbishops = state.bitboards[b];
    BB copy_bb;

    score += material[b] * pop_count(wbishops);
    score -= material[b] * pop_count(bbishops);

    copy_bb = wbishops;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score += bishop_psqt[flip_rank(sq)];
    }
    
    copy_bb = bbishops;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score -= bishop_psqt[sq];
    }

    return score;
}

Score Board::eval_rooks() {
    Score score = 0;

    BB wrooks = state.bitboards[R];
    BB brooks = state.bitboards[r];
    BB copy_bb;

    score += material[r] * pop_count(wrooks);
    score -= material[r] * pop_count(brooks);

    copy_bb = wrooks;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score += rook_psqt[flip_rank(sq)];
    }
    
    copy_bb = brooks;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score -= rook_psqt[sq];
    }

    // Rooks on open files.
    BB wpawns = state.bitboards[P];
    BB bpawns = state.bitboards[p];
    int num_rooks_on_open = pop_count(open_file(wpawns, bpawns) & wrooks);
    score += OPEN_FILE_ROOKS_BONUS * num_rooks_on_open;

    num_rooks_on_open = pop_count(open_file(wpawns, bpawns) & brooks);
    score -= OPEN_FILE_ROOKS_BONUS * num_rooks_on_open;

    // Rooks on semi-open files.
    int num_rooks_on_semi_open = pop_count(w_half_open_files(wpawns, bpawns) & wrooks);
    score += HALF_OPEN_FILE_ROOKS_BONUS * num_rooks_on_semi_open;

    num_rooks_on_semi_open = pop_count(b_half_open_files(wpawns, bpawns) & brooks);
    score -= HALF_OPEN_FILE_ROOKS_BONUS * num_rooks_on_semi_open;

    return score;
}

Score Board::eval_queens() {
    Score score = 0;

    BB wqueens = state.bitboards[Q];
    BB bqueens = state.bitboards[q];
    BB copy_bb;

    score += material[q] * pop_count(wqueens);
    score -= material[q] * pop_count(bqueens);

    copy_bb = wqueens;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score += queen_psqt[flip_rank(sq)];
    }
    
    copy_bb = bqueens;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score -= queen_psqt[sq];
    }

    return score;
}

Score Board::eval_kings() {

    bool is_endgame = 
    pop_count(state.bitboards[allpieces] ^ (state.bitboards[p] | state.bitboards[P] | state.bitboards[k] | state.bitboards[K]))
    <= 7;
    
    Score score = 0;

    BB wkings = state.bitboards[K];
    BB bkings = state.bitboards[k];
    BB copy_bb;

    int table = is_endgame ? 1 : 0;

    copy_bb = wkings;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score += king_psqt[table][flip_rank(sq)];
    }
    
    copy_bb = bkings;
    while (copy_bb) {
        Square sq = pop_lsb(copy_bb);
        score -= king_psqt[table][sq];
    }

    return score;
}

Score Board::eval() {
    Score score = 0;

    score += eval_pawns();
    score += eval_knights();
    score += eval_bishops();
    score += eval_rooks();
    score += eval_queens();
    score += eval_kings();

    return (state.side_to_move == white) ? score : -score;
}