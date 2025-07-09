#include "../include/board.hpp"
#include "../include/misc.hpp"
#include <array>

using namespace bitboard_utils;
using namespace std;

constexpr array<Score, 5> material = { 100, 320, 330, 500, 900 };

constexpr array<Score, 64> pawn_psqt = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    2, -5,-10, -2, -2,-10, -5,  2,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

constexpr array<Score, 64> knight_psqt = {
    -50,-20,-30,-30,-30,-30,-20,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  7, 13, 10, 10, 13,  7,-30,
    -30,  2, 12, 17, 17, 12,  2,-30,
    -30,  0, 12, 17, 17, 12,  0,-30,
    -30,  7, 13, 10, 10, 13,  7,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-20,-30,-30,-30,-30,-20,-50
};

constexpr array<Score, 64> bishop_psqt = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 12, 10, 10, 12,  0,-10,
    -10, 10,  7, 12, 12,  7, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

constexpr array<Score, 64> rook_psqt = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,   0,  5,  5,  0,  0,  0
};

constexpr array<Score, 64> queen_psqt = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

constexpr array<array<Score, 64>, 2> king_psqt = {{
    {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20, -5, -5, -5, -5, 20, 20,
        20, 30, 10,  0,  0,  7, 27, 17
    },
    {
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
    }
}};

Score Board::eval_pawns() {
    Score score = 0;

    BB wpawns = state.bitboards[P];
    BB bpawns = state.bitboards[p];
    BB copy_bb;

    score += material[p] * pop_count(wpawns);  
    score -= material[p] * pop_count(bpawns);          
    
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