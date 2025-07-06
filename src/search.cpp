#include "../include/search.hpp"
#include "../include/board.hpp"
#include "../include/utils.hpp"
#include "../include/transposition.hpp"
#include <iostream>
#include <array>

const SearchParams* g_search_params = nullptr;

constexpr std::array<std::array<int, 6>, 5> mva_lva_table = {{
    {{ 15, 14, 13, 12, 11, 10 }},
    {{ 25, 24, 23, 22, 21, 20 }},
    {{ 35, 34, 33, 32, 31, 30 }},
    {{ 45, 44, 43, 42, 41, 40 }},
    {{ 55, 54, 53, 52, 51, 50 }}
}};

void Board::order_moves(Move hash_move) {
    auto& list = state.move_list;
    const int pv_index = get_pv_index(search_state.ply);

    std::vector<std::pair<Move, int>> scored_moves(list.size());
    Move pv_move = search_state.pv_table[pv_index];
    Move killer_0 = search_state.killer_moves[search_state.ply][0];
    Move killer_1 = search_state.killer_moves[search_state.ply][1];

    // Score each move using transform
    std::transform(list.begin(), list.end(), scored_moves.begin(),
        [&](Move move) -> std::pair<Move, int> {
            int from = get_from_sq(move);
            int to = get_to_sq(move);
            int score = 0;

            if (move == hash_move)
                score = 1000;
            else if (move == pv_move)
                score = 900;
            else if (get_code(move) >= npromo)
                score = 800;
            else if (is_move(move, capture)) {
                int attacker = state.piece_list[from];
                if (attacker > 5) attacker -= 6;
                int victim = state.piece_list[to];
                if (victim > 5) victim -= 6;
                score = 800 + mva_lva_table[victim][attacker];
            }
            else if (move == killer_0)
                score = 700;
            else if (move == killer_1)
                score = 600;
            else
                score = search_state.history_moves[from][to][state.side_to_move];

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
    if (stop_flag)
        return true;
    if (!g_search_params) // fallback, should not happen
        return false;

    // Time check
    if (g_search_params->move_time > 0) {
        int elapsed = elapsed_ms(search_state.start_time);
        if (elapsed >= g_search_params->move_time)
            return true;
    }

    // Node check
    if (g_search_params->nodes > 0 && search_state.nodes >= g_search_params->nodes)
        return true;
    
    // Depth check (current depth = root depth - ply)
    if (g_search_params->max_depth > 0 && (g_search_params->max_depth - search_state.ply) <= 0)
        return true;

    return false;
}

int Board::quiescence(int alpha, int beta) {
    int stand_pat = game_board.eval();
    int best_val = stand_pat;
    if (stand_pat >= beta) return stand_pat;
    if (stand_pat > alpha) alpha = stand_pat;

    game_board.generate_moves<CAPTURES>();
    game_board.order_moves(nullmove);

    if (game_board.is_draw()) return 0;
    if (game_board.winner() == opposition_colour(game_board.state.side_to_move)) return -INF;

    if (is_search_stopped()) return best_val;

    for (Move move : game_board.state.move_list) {
        game_board.make_move(move);
        search_state.ply++;
        search_state.nodes++;

        int score = -quiescence(-beta, -alpha);

        search_state.ply--;
        game_board.unmake_last_move();

        if (score >= beta) return score;
        if (is_search_stopped()) break;

        if (score > best_val) best_val = score;
        if (score > alpha) alpha = score;
    }

    return best_val;
}

int Board::search(int depth, int alpha, int beta) {
    if (depth <= 0)
        return quiescence(alpha, beta);

    int score = 0;
    int static_eval = game_board.eval();
    int old_alpha = alpha;
    int best_score = -INF;
    long moves_searched = 0;
    EntryType node_type = UPPER;    

    int pv_idx = get_pv_index(search_state.ply);
    int next_pv_idx = get_next_pv_index(search_state.ply);

    if (is_search_stopped()) return alpha;
    
    // Transposition Table Cut-offs
    TranspositionEntry *entry = game_table->probe(state.hash_key, depth);
    if (entry && search_state.pv_table[pv_idx] != 0) {
        if ((entry->type == EXACT) 
        || (entry->type == LOWER && entry->score >= beta) 
        || (entry->type == UPPER && entry->score < alpha))
            return entry->score;
    }

    bool can_prune = (depth <= 2) && !is_in_check;
    int pruning_margin = 125 * depth * depth;

    // Razoring
    if (can_prune && static_eval + pruning_margin <= alpha)
        return quiescence(alpha, beta);

    // Generate and sort moves
    game_board.generate_moves<ALLMOVES>();
    game_board.order_moves(entry ? entry->hash_move : nullmove);

    // Handle game over
    if (game_board.is_draw()) return 0;
    if (game_board.winner() == opposition_colour(game_board.state.side_to_move)) return -INF;

    if (search_state.ply+1 >= MAX_PLY) return static_eval;
    
    // Null move reduction
    if (!is_in_check) {
        int reduction = depth > 6 ? 4 : 3;
        game_board.make_move(nullmove);
        search_state.ply++;
        search_state.nodes++;
        score = -search(depth - reduction - 1, 0-beta, 1-beta);
        search_state.ply--;
        game_board.unmake_last_move();
        if (score >= beta) {
            depth -= 4;
            if (depth <= 0)
                return quiescence(alpha, beta);
        }
    }
    
    for (Move move : game_board.state.move_list) {
        game_board.make_move(move);
        search_state.ply++;
        search_state.nodes++;


        // Futility pruning
        if (can_prune && (node_type == EXACT) && !is_move(move, capture) && get_code(move) < npromo && !is_in_check) {
            if (eval() + pruning_margin <= alpha) {
                search_state.ply--;
                game_board.unmake_last_move();
                continue; // Prune the move
            }
        }

        moves_searched++;

        if (moves_searched == 1 || is_move(move, capture) || get_code(move) >= npromo || is_in_check)
            // Do normal search if searching first few moves
            score = -search(depth - 1,  -beta, -alpha);

        else {
            
            int reduction = 1;

            if (moves_searched >= 3) {
                // Late move reductions
                if (moves_searched < 6) reduction = 2;
                if (moves_searched >= 6) reduction = 3;
            }

            int reduced_depth = std::max(depth - reduction, 1);

            // PVS search
            score = -search(reduced_depth, -(alpha + 1), -alpha);

            // Research
            if (score > alpha && beta - alpha > 1)
                score = -search(depth - 1, -beta, -alpha);
        }

        search_state.ply--;
        int ply = search_state.ply;

        game_board.unmake_last_move();

        if (is_search_stopped()) break;

        if (score > best_score && search_state.ply == 0) {
            best_score = score;
            search_state.fallback_move = move;
        }

        if (score >= beta) {
            node_type = LOWER;
            if (!is_move(move, capture)) {
                // Update killer moves
                if (search_state.killer_moves[ply][1] == nullmove) {
                    search_state.killer_moves[ply][0] = search_state.killer_moves[ply][1];
                    search_state.killer_moves[ply][1] = move;
                } else
                    search_state.killer_moves[ply][0] = move;
            } else {
                int from = get_from_sq(move);
                int to = get_to_sq(move);
                search_state.history_moves[from][to][game_board.state.side_to_move] = depth * depth;
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

void Board::run_search(SearchParams params) {
    g_search_params = &params;
    int alpha = -INF, beta = INF;
    search_state.start_time = std::chrono::steady_clock::now();
    int d = 1;

    // Aspiration windows vars
    int widening = 75, aw_fails_alpha = 0, aw_fails_beta = 0;

    // Decay history heuristic.
    for (int side = 0; side < 2; side++)
        for (int from = 0; from < 64; from++)
            for (int to = 0; to < 64; to++)
                search_state.history_moves[from][to][side] /= 2;

    std::chrono::steady_clock::time_point depth_search_time = std::chrono::steady_clock::now();
    while (1) {
        search_state.nodes = 0;

        int score = search(d, alpha, beta);
        if (is_search_stopped()) break;

        // AW researches
        if (score <= alpha) {
            aw_fails_alpha++;
            alpha -= widening * aw_fails_alpha;
            continue;
        }

        if (score >= beta) {
            aw_fails_beta++;
            beta += widening * aw_fails_beta;
            continue;
        }

        aw_fails_alpha = 0;
        aw_fails_beta = 0;

        // Apply AW
        alpha = score - 25;
        beta = score + 25;

        std::cout << "info depth " << d << " nodes " << search_state.nodes << " time " << elapsed_ms(depth_search_time) << " score cp " << score << " pv ";
        for (int i = 0; i < search_state.pv_length[0]; i++) {
            print_move(search_state.pv_table[i]);
            std::cout << " ";
        }

        std::cout << std::endl;
        
        depth_search_time = std::chrono::steady_clock::now();
        d++;
        if (d > params.max_depth && params.max_depth > 0) break;
    }

    std::cout << "bestmove ";
    if (search_state.pv_table[0] == nullmove)
        print_move(search_state.fallback_move);
    else
        print_move(search_state.pv_table[0]);
    std::cout << std::endl;
    g_search_params = nullptr;
    stop_flag = true;
}