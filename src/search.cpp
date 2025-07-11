#include "../include/search.hpp"
#include "../include/board.hpp"
#include "../include/utils.hpp"
#include "../include/transposition.hpp"
#include <iostream>
#include <algorithm>
#include <array>
#include <cassert>

long researches = 0;
long tt_cuttoffs = 0;

/// @brief Values for scoring captures. See https://www.chessprogramming.org/MVV-LVA.
constexpr std::array<std::array<int, 6>, 5> MVV_LVA_table = {{
    {{ 15, 14, 13, 12, 11, 10 }},
    {{ 25, 24, 23, 22, 21, 20 }},
    {{ 35, 34, 33, 32, 31, 30 }},
    {{ 45, 44, 43, 42, 41, 40 }},
    {{ 55, 54, 53, 52, 51, 50 }}
}};

void Board::order_moves(Move hash_move) {
    auto& list = state.move_list;
    const int pv_index = get_pv_index(search_state.ply);

    std::vector<std::pair<Move, Score>> scored_moves(list.size());
    Move pv_move = search_state.pv_table[pv_index];
    Move killer_0 = killer_moves[search_state.ply][0];
    Move killer_1 = killer_moves[search_state.ply][1];

    /*
    Ranking of moves is as follows:
    1. Hash moves (from transposition table).
    2. PV move.
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

            if (move == hash_move)
                score = 1000;
            else if (move == pv_move)
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

bool Board::is_search_stopped() {
    if (stop_flag.load())
        return true;

    // Time check
    if (search_params.move_time > 0) {
        int elapsed = elapsed_ms(search_state.start_time);
        if (elapsed >= search_params.move_time)
            return true;
    }

    // Node check
    if (search_params.nodes > 0 && search_state.nodes >= search_params.nodes)
        return true;
    
    // Depth check (current depth = root depth - ply)
    if (search_params.max_depth > 0 && search_state.depth > search_params.max_depth)
        return true;

    if (search_state.depth > MAX_DEPTH && !search_params.infinite)
        return true;

    if (search_state.ply >= MAX_PLY) return true;

    return false;
}

// See https://www.chessprogramming.org/Quiescence_Search.
Score Board::quiescence(Score alpha, Score beta) {
    Score stand_pat = game_board.eval();
    Score best_val = stand_pat;
    if (stand_pat >= beta) return stand_pat;
    if (stand_pat > alpha) alpha = stand_pat;

    generate_moves<ALLMOVES>();
    order_moves(nullmove);
    if (is_draw()) return 0;
    if (winner() == opposition_colour(state.side_to_move)) return -INF;
    
    if (is_search_stopped()) return best_val;
    
    for (Move move : state.move_list) {
        if (!is_move_capture(move)) continue;
        make_move(move);
        search_state.ply++;
        search_state.nodes++;

        Score score = -quiescence(-beta, -alpha);

        search_state.ply--;
        unmake_last_move();

        if (score >= beta) return score;
        if (is_search_stopped()) break;

        if (score > best_val) best_val = score;
        if (score > alpha) alpha = score;
    }

    return best_val;
}

// See https://www.chessprogramming.org/Negamax.
Score Board::search(int depth, Score alpha, Score beta) {
    if (depth <= 0)
        return quiescence(alpha, beta); 

    search_state.depth = depth;
    Score score = 0;
    Score static_eval = game_board.eval();
    Score old_alpha = alpha;
    Score best_score = -INF;
    long moves_searched = 0;
    EntryType node_type = UPPER;    

    int pv_idx = get_pv_index(search_state.ply);
    int next_pv_idx = get_next_pv_index(search_state.ply);

    // Transposition Table Cut-offs
    TranspositionEntry *entry = game_table->probe(state.hash_key, depth);
    if (entry && search_state.pv_table[pv_idx] != 0) {
        if ((entry->type == EXACT) 
        || (entry->type == LOWER && entry->score >= beta) 
        || (entry->type == UPPER && entry->score < alpha)) {
            tt_cuttoffs++;
            return entry->score;
        }
    }
    
    bool can_prune = (depth <= 2) && !is_in_check;
    Score pruning_margin = 125 * depth * depth;
    
    // Razoring. See https://www.chessprogramming.org/Razoring.
    if (can_prune && static_eval + pruning_margin <= alpha)
        return quiescence(alpha, beta);
    
    // Generate and sort moves
    generate_moves<ALLMOVES>();
    order_moves(entry ? entry->hash_move : nullmove);
    
    // Handle game over
    if (is_draw()) return 0;
    if (winner() == opposition_colour(game_board.state.side_to_move)) return -INF;

    if (is_search_stopped()) return alpha;

    // Null move reduction. See https://www.chessprogramming.org/Null_Move_Reductions.
    if (!is_in_check) {
        int reduction = depth > 6 ? 4 : 3;
        make_move(nullmove);
        search_state.ply++;
        search_state.nodes++;
        score = -search(depth - reduction - 1, -beta, -(beta-1));
        search_state.ply--;
        unmake_last_move();
        if (score >= beta)
            return score;
    }
    
    for (Move move : game_board.state.move_list) {
        make_move(move);
        
        search_state.ply++;
        search_state.nodes++;

        // Futility pruning. See https://www.chessprogramming.org/Futility_Pruning.
        if (can_prune && (node_type == EXACT) && !is_move(move, capture) && get_code(move) < npromo) {
            if (eval() + pruning_margin <= alpha) {
                search_state.ply--;
                unmake_last_move();
                continue; // Prune the move
            }
        }

        moves_searched++;

        if (moves_searched == 1 || is_move(move, capture) || get_code(move) >= npromo)
            // Do normal search if searching first few moves
            score = -search(depth - 1,  -beta, -alpha);

        else {
            
            int reduction = 1;
            
            // Late move reductions. See https://www.chessprogramming.org/Late_Move_Reductions.
            if (moves_searched >= 3 && depth >= 3) {
                reduction = std::max(int(moves_searched / 2 - 1), 2);
            }

            int reduced_depth = std::max(depth - reduction, 1);

            // PVS search. See https://www.chessprogramming.org/Principal_Variation_Search.
            score = -search(depth - 1, -(alpha + 1), -alpha);

            // Research 
            if (score > alpha && beta - alpha > 1) {
                // beta - alpha > 1 prevents redundant research of Non-PV nodes.

                researches++;
                score = -search(depth - 1, -beta, -alpha);
            }
        }

        search_state.ply--;
        int ply = search_state.ply;

        unmake_last_move();

        if (is_search_stopped()) break;

        if (score > best_score && search_state.ply == 0) {
            best_score = score;
            search_state.fallback_move = move;
        }

        if (score >= beta) {
            node_type = LOWER;
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
                history_moves[from][to][game_board.state.side_to_move] = depth * depth;
            }

            // Add new entry
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
            alpha = score;
            node_type = EXACT;
            search_state.pv_table[pv_idx] = move;

            if (search_state.pv_length[ply + 1] > 0) {
                for (int j = 0; j < search_state.pv_length[ply + 1]; j++)
                    search_state.pv_table[pv_idx + j + 1] = search_state.pv_table[next_pv_idx + j];
                search_state.pv_length[ply] = search_state.pv_length[ply + 1] + 1;
            } else 
                search_state.pv_length[ply] = 1;
        }
    }

    if (moves_searched) {
        TranspositionEntry new_entry;
        new_entry.depth = depth;
        new_entry.hash_move = search_state.pv_table[pv_idx];
        new_entry.key = state.hash_key;
        new_entry.score = alpha;
        if (alpha <= old_alpha)
            new_entry.type = UPPER;
        else if (alpha >= beta)
            new_entry.type = LOWER;
        else
            new_entry.type = EXACT;
        game_table->store_entry(new_entry);
    }

    return alpha;
}

void Board::run_search() {
    Score alpha = -INF, beta = INF;
    search_state.start_time = std::chrono::steady_clock::now();
    int d = 1;

    // Aspiration windows vars. See https://www.chessprogramming.org/Aspiration_Windows.
    Score widening = 100, aw_fails_alpha = 0, aw_fails_beta = 0;

    // Decay history heuristic.
    for (int side = 0; side < 2; side++)
        for (int from = 0; from < 64; from++)
            for (int to = 0; to < 64; to++)
                history_moves[from][to][side] /= 2;

    std::chrono::steady_clock::time_point depth_search_time = std::chrono::steady_clock::now();
    Score score;
    array<Score, SEARCH_HELPER_THREADS> thread_scores {};
    while (1) {
        researches = 0;
        tt_cuttoffs = 0;
        search_state.nodes = 0;
        search_state.ply = 0;

        score = search(d, alpha, beta);

        if (is_search_stopped()) break;

        // AW researches
        if (score <= alpha) {
            aw_fails_alpha++;
            alpha -= widening;
            beta = score + widening;
            continue;
        }

        if (score >= beta) {
            aw_fails_beta++;
            beta += widening;
            alpha = score - widening;
            continue;
        }

        aw_fails_alpha = 0;
        aw_fails_beta = 0;

        // Apply AW
        alpha = score - 25;
        beta = score + 25;

        std::cout << "info depth " << d << " nodes " << search_state.nodes << " time " 
        << elapsed_ms(depth_search_time) << " researches " << researches << " tt_cuttoffs " 
        << tt_cuttoffs << " score cp " << score << " pv ";
        for (int i = 0; i < search_state.pv_length[0]; i++) {
            print_move(search_state.pv_table[i]);
            std::cout << " ";
        }

        std::cout << std::endl;
        
        depth_search_time = std::chrono::steady_clock::now();
        d++;
        
        if (d > search_params.max_depth && search_params.max_depth > 0) break;
    }

    std::cout << "bestmove ";
    if (search_state.pv_table[0] == nullmove)
        print_move(search_state.fallback_move);
    else
        print_move(search_state.pv_table[0]);
    std::cout << std::endl;
    stop_flag.store(true);
}