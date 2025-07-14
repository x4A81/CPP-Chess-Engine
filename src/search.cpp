#include "../include/search.hpp"
#include "../include/board.hpp"
#include "../include/utils.hpp"
#include "../include/transposition.hpp"
#include "../include/book.hpp"
#include <iostream>
#include <random>

/// @brief Values for scoring captures. See https://www.chessprogramming.org/MVV-LVA.
constexpr std::array<std::array<int, 6>, 5> MVV_LVA_table = {{
    {{ 15, 14, 13, 12, 11, 10 }},
    {{ 25, 24, 23, 22, 21, 20 }},
    {{ 35, 34, 33, 32, 31, 30 }},
    {{ 45, 44, 43, 42, 41, 40 }},
    {{ 55, 54, 53, 52, 51, 50 }}
}};

void Board::clean_search() {
    history_moves.fill({{0}});
    prev_pv_table.fill(nullmove);
    killer_moves.fill({{nullmove}});
    pv_table.fill(nullmove);
    pv_length.fill(0);
    stop_flag.store(true);
}

void Board::update_pv(int ply, int pv_idx, int next_pv_idx) {
    if (pv_length[ply + 1] > 0) {
        for (int n = 0; n < pv_length[ply + 1]; n++) {
            if (pv_table[next_pv_idx + n] == nullmove) break;
            pv_table[pv_idx + n + 1] = pv_table[next_pv_idx + n];
            pv_length[ply] = pv_length[ply + 1] + 1;
        }

    } else pv_length[ply] = 1;
}

bool Board::is_search_stopped() {
    if (stop_flag.load())
        return true;

    // Time check
    if (search_params.move_time > 0) {
        int elapsed = elapsed_ms(start_time);
        if (elapsed >= search_params.move_time - 50)
            // Subtract 50 ms for some calc time.
            return true;
    }

    return false;
}

void Board::order_moves(Move hash_move, int ply) {
    MoveList& list = state.move_list;
    const int pv_index = get_pv_index(ply);

    std::vector<std::pair<Move, Score>> scored_moves(list.size());
    Move pv_move = prev_pv_table[pv_index];
    Move killer_0 = killer_moves[ply][0];
    Move killer_1 = killer_moves[ply][1];

    /*
    Ranking of moves is as follows:
    1. PV move.
    2. Hash moves (from transposition table).
    3. Promotions.
    4. Captures using MVV-LVA. i.e PxR is ranked higher than BxR
    5. Killer moves.
    6. History moves.
    */

    // Score each move using transform
    std::transform(list.begin(), list.end(), scored_moves.begin(),
        [&](Move move) -> std::pair<Move, Score> {
            Square from = get_from_sq(move);
            Square to = get_to_sq(move);
            Score score = 0;

            if (move == pv_move)
                score = 1000;
            else if (move == hash_move)
                score = 900;
            else if (get_code(move) >= npromo)
                score = 800;
            else if (is_move_capture(move)) {
                Piece attacker = state.piece_list[from];
                if (attacker > 5) attacker -= 6;
                Piece victim = state.piece_list[to];
                if (victim > 5) victim -= 6;
                score = 800 + MVV_LVA_table[victim][attacker];
            }
            else if (move == killer_0)
                score = 700;
            else if (move == killer_1)
                score = 600;
            else
                score = history_moves[from][to][state.side_to_move];

            return {move, score};
        });

    // Sort by score descending
    std::sort(scored_moves.begin(), scored_moves.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });

    std::transform(scored_moves.begin(), scored_moves.end(), list.begin(),
               [](const auto& pair) { return pair.first; });
}

// See https://www.chessprogramming.org/Quiescence_Search.
Score Board::quiescence(Score alpha, Score beta, int ply) {

    Score best_val = alpha;
    bool check_flag = true;
    if (!is_side_in_check(state.side_to_move)) {
        check_flag = false;
        Score stand_pat = eval();
        best_val = stand_pat;
        if (stand_pat >= beta) return stand_pat;
        if (stand_pat > alpha) alpha = stand_pat;
      //  generate_moves<CAPTURES>(); // BUGS IN CAPTURE GEN
    }

    //else
    generate_moves<ALLMOVES>();

    order_moves(nullmove, ply);
    
    if (is_search_stopped()) return best_val; 
    
    for (Move move : state.move_list) {
        if (!is_move_capture(move) && !check_flag) continue;
        make_move(move);
        nodes++;

        Score score = -quiescence(-beta, -alpha, ply + 1);

        unmake_last_move();

        if (score >= beta) return score;
        
        if (score > best_val) best_val = score;
        if (score > alpha) alpha = score;
        if (is_search_stopped()) break;
    }

    return best_val;
}

// See https://www.chessprogramming.org/Negamax.
Score Board::search_root(int depth, Score alpha, Score beta) {
    Score score = 0;
    int moves_searched = 0;
    EntryType ent = UPPER;
    Score old_alpha = alpha;

    generate_moves<ALLMOVES>();
    order_moves(nullmove, 0);

    for (Move move : state.move_list) {
        make_move(move);

        // Principle Variation Search at root. Left most node is always PV_node
        if (moves_searched == 0)
            score = -search(depth - 1, 1, -beta, -alpha, true);
        else {
            score = -search(depth - 1, 1, -alpha - 1, -alpha, false);

            if (score > alpha)
                score = -search(depth - 1, 1, -beta, -alpha, true);
        }

        unmake_last_move();
        if (score >= beta) {
            if (!is_move_capture(move)) {
                // Update killer heuristics. See https://www.chessprogramming.org/Killer_Heuristic.
                if (killer_moves[0][1] == nullmove) {
                    killer_moves[0][0] = killer_moves[0][1];
                    killer_moves[0][1] = move;
                } else
                    killer_moves[0][0] = move;
            } else {
                // Update history heuristic. See https://www.chessprogramming.org/History_Heuristic.
                Square from = get_from_sq(move);
                Square to = get_to_sq(move);
                history_moves[from][to][state.side_to_move] = depth * depth;
            }

            TranspositionEntry new_entry;
            new_entry.depth = depth;
            new_entry.hash_move = move;
            new_entry.key = state.hash_key;
            new_entry.score = beta;
            new_entry.type = LOWER;
            game_table->store_entry(new_entry);

            return beta;
        }

        if (score > alpha) {
            pv_table[0] = move;
            update_pv(0, 0, MAX_PLY);
            alpha = score;
            ent = EXACT;
        }

        if (is_search_stopped()) return alpha;
    }

    if (alpha <= old_alpha)
        ent = UPPER;
    else if (alpha >= beta)
        ent = LOWER;
    else
        ent = EXACT;

    TranspositionEntry new_entry;
    new_entry.depth = depth;
    new_entry.hash_move = pv_table[0];
    new_entry.key = state.hash_key;
    new_entry.score = alpha;
    new_entry.type = ent;
    game_table->store_entry(new_entry);
    
    return alpha;
}

Score Board::search(int depth, int ply, Score alpha, Score beta, bool is_pv_node, bool null_move_allowed) {

    // Check extensions
    if (is_side_in_check(state.side_to_move)) depth++;

    // Also ensure that quiescence is not called when in check

    if (depth <= 0)
        return quiescence(alpha, beta, ply);

    int pv_idx = get_pv_index(ply);
    int next_pv_idx = get_next_pv_index(ply);

    // Handle upcoming repetitions.
    if (is_rep())
        return 0;

    // Transposition Table Cut-offs
    TranspositionEntry *entry = game_table->probe(state.hash_key, depth);

    // Ensures that pv is not shortened
    if (entry != nullptr && pv_table[pv_idx] != nullmove) {

        // In pv nodes, only return if hit is exact.
        EntryType ent = entry->type;
        Score ents = entry->score;
        ents = score_from_tt(ents, ply);
        if (!is_pv_node) {
            if (ent == EXACT ||
            (ent == LOWER && ents >= beta) ||
            (ent == UPPER && ents < alpha))
                return ents;
        } else if (ent == EXACT)
            return ents;
    }

    generate_moves<ALLMOVES>();
    order_moves(entry ? entry->hash_move : nullmove, ply);
    
    Score score = 0;

    long moves_searched = 0;
    EntryType tt_type = UPPER;
    bool raised_alpha = false;
    int new_depth = depth - 1;
    Move best_move = nullmove;
    pv_table[pv_idx] = nullmove;
    pv_length[ply] = 0;
    Score old_alpha = alpha;
    Score static_eval = eval();

    // Null Move Pruning
    bool only_king_and_pawns = 
    (state.bitboards[allpieces] ^ (state.bitboards[p] | state.bitboards[P] | state.bitboards[k] | state.bitboards[K]))
    == 0;
    bool do_null_pruning = 
    !state.is_in_check && !only_king_and_pawns && null_move_allowed
    && (static_eval >= beta);

    if (do_null_pruning) {
        int R = depth > 6 ? 4 : 3;
        make_move(nullmove);
        score = -search(depth - R, ply + 1, -beta, -beta + 1, false, false);
        unmake_last_move();
        if (score >= beta) return score;
    }

    bool f_prune = 
    (depth < 3) && !state.is_in_check && !is_pv_node && (abs(alpha) < MATE_VALUE)
    && (static_eval + (FUTILITY_MARGIN * depth * depth) <= alpha);

    for (Move move : state.move_list) {
        make_move(move);
        
        // Futility Pruning
        if (f_prune && !is_side_in_check(state.side_to_move) 
        && is_move(move, capture) && (get_code(move) < npromo)) {
            unmake_last_move();
            continue;
        }
        
        // Late Move Pruning
        int reduction = 0;
        if (!is_pv_node && new_depth > 3 && moves_searched > 3
            && !is_move(move, capture) && get_code(move) < npromo && !state.is_in_check) {
            if (moves_searched < 6) reduction += 3;
            if (moves_searched > 8) reduction += 4;
            new_depth -= reduction;
        }

        if (!raised_alpha)
            score = -search(new_depth, ply + 1, -beta, -alpha, is_pv_node);
        
        else {
            score = -search(new_depth, ply + 1, -alpha - 1, -alpha, false);

            if (score > alpha)
                score = -search(new_depth, ply + 1, -beta, -alpha, true);
        }

        // Research LMR
        if (reduction && score > alpha) {
            new_depth += reduction;
            reduction = 0;

            if (!raised_alpha)
                score = -search(new_depth, ply + 1, -beta, -alpha, is_pv_node);
            
            else {
                score = -search(new_depth, ply + 1, -alpha - 1, -alpha, false);

                if (score > alpha)
                    score = -search(new_depth, ply + 1, -beta, -alpha, is_pv_node);
            }
        }
        
        nodes++;
        moves_searched++;

        unmake_last_move();

        if (score >= beta) {
            if (!is_move_capture(move)) {
                // Update killer heuristics. See https://www.chessprogramming.org/Killer_Heuristic.
                if (killer_moves[ply][1] == nullmove) {
                    killer_moves[ply][0] = killer_moves[ply][1];
                    killer_moves[ply][1] = move;
                } else
                    killer_moves[ply][0] = move;
            } else {
                // Update history heuristic. See https://www.chessprogramming.org/History_Heuristic.
                Square from = get_from_sq(move);
                Square to = get_to_sq(move);
                history_moves[from][to][state.side_to_move] = depth * depth;
            }
            
            TranspositionEntry new_entry;
            new_entry.depth = depth;
            new_entry.hash_move = move;
            new_entry.key = state.hash_key;
            new_entry.score = beta;
            new_entry.type = LOWER;
            game_table->store_entry(new_entry);
            return beta;
        }

        if (score > alpha) {
            pv_table[pv_idx] = move;
            best_move = move;
            update_pv(ply, pv_idx, next_pv_idx);
            raised_alpha = true;
            tt_type = EXACT;
            alpha = score;
        }

        if (is_search_stopped()) break;
    }

    if (state.move_list.is_empty()) {
        if (state.is_in_check) alpha = -MATE_VALUE + ply;
        else alpha = -eval() / 3;
    }

    if (alpha <= old_alpha)
        tt_type = UPPER;
    else if (alpha >= beta)
        tt_type = LOWER;
    else
        tt_type = EXACT;

    TranspositionEntry new_entry;
    new_entry.depth = depth;
    new_entry.hash_move = best_move;
    new_entry.key = state.hash_key;
    new_entry.score = score_to_tt(alpha, ply);
    new_entry.type = tt_type;
    game_table->store_entry(new_entry);

    return alpha;
}

void Board::run_search() {
    // First try the opening book
    vector<polyglot::BookEntry> entries = polyglot::probe_book(polyglot::gen_poly_key(game_board.state));

    if (!entries.empty()) {
        cout << "info string book move " << endl;
        cout << "bestmove ";
        int limit = std::min(3, (int)entries.size());
        int idx = rand() % limit;
        print_move(polyglot::get_book_move(entries[idx], game_board.state));
        cout << endl;
        return;
    }

    if (search_params.move_time == UNUSED && search_params.max_depth == UNUSED && !search_params.infinite) {
        search_params.move_time = (state.side_to_move == white) ? search_params.wtime : search_params.btime;
        search_params.move_time /= search_params.movestogo;
        search_params.max_depth = MAX_DEPTH;
        cout << "info string searching for " << search_params.move_time << "ms" << endl;
    }

    prev_pv_table = pv_table;
    pv_table.fill(nullmove);
    pv_length.fill(0);
    Score alpha = -INF, beta = INF;
    start_time = std::chrono::steady_clock::now();
    int d = 1;

    // Aspiration windows vars. See https://www.chessprogramming.org/Aspiration_Windows.

    // Decay history heuristic.
    for (int side = 0; side < 2; side++)
        for (int from = 0; from < 64; from++)
            for (int to = 0; to < 64; to++)
                history_moves[from][to][side] /= 2;

    std::chrono::steady_clock::time_point depth_search_time = std::chrono::steady_clock::now();
    Score score;
    bool aw_research = false;
    int fail_lows = 0;
    int fail_highs = 0;
    while (1) {
        nodes = 0;

        if (d > 1 && !aw_research) {
            // Aspiration Windows
            alpha = score - 25;
            beta = score + 25;
        }

        score = search_root(d, alpha, beta);

        // Research Aspiration Windows with gradual widening.
        aw_research = false;
        if (score <= alpha) {
            fail_lows++;
            alpha = score - (50 * fail_lows);
            aw_research = true;
        }

        if (score >= beta) {
            fail_highs++;
            beta = beta + (50 * fail_highs);
            aw_research = true;
        }

        if (is_search_stopped()) break;

        if (aw_research) continue;

        fail_highs = 0;
        fail_lows = 0;
        
        std::cout << "info depth " << d << " nodes " << nodes << " time " << elapsed_ms(depth_search_time); 
        
        if (abs(score) < MATE_VALUE - 100)
            cout << " score cp " << score;
        else {
            cout << " score mate ";

            // Mate is printed in moves not plies, hence halving and +/- 1.
            if (score > 0)
                cout << (MATE_VALUE - score) / 2 + 1;
            else
                cout << -(MATE_VALUE + score) / 2 - 1;
        }

        cout << " pv ";
        for (int i = 0; i < pv_length[0]; i++) {
            print_move(pv_table[i]);
            std::cout << " ";
        }

        std::cout << std::endl;
        
        depth_search_time = std::chrono::steady_clock::now();
        d++;
        prev_pv_table = pv_table;
        if (d > search_params.max_depth && search_params.max_depth > 0) break;
        pv_table.fill(nullmove);
        pv_length.fill(0);
    }

    std::cout << "bestmove ";
    print_move(prev_pv_table[0]);
    std::cout << std::endl;
    stop_flag.store(true);
}