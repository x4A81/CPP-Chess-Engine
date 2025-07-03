#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "board.hpp"
#include "transposition.hpp"
#include <array>
#include <atomic>
#include <chrono>

#define MAX_PLY 64
#define MAX_DEPTH 64
#define PV_TABLE_SIZE (MAX_PLY*MAX_PLY+MAX_PLY)/2
#define INF 100000

inline std::atomic<bool> stop_flag;
inline Transposition transposition_table;

struct SearchParams {
    int start_depth = 1;
    int max_depth = 0;
    int nodes = 0;
    int move_time = 0;
    int inc = 0;
    bool infinite = false;
    bool ponder = false;
};

struct SearchState {
    std::chrono::steady_clock::time_point start_time;
    std::array<Move, PV_TABLE_SIZE> pv_table;
    std::array<int, MAX_PLY> pv_length;
    std::array<std::array<Move, 2>, MAX_PLY> killer_moves;
    Move fallback_move = nullmove;
};

inline int pv_index(int ply) { return (ply*(2*MAX_PLY+1-ply))/2; }
inline int next_pv_index(int ply) { return pv_index(ply) + MAX_PLY - ply; }
inline int elapsed_ms(const SearchState& vars) {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        steady_clock::now() - vars.start_time
    ).count();
}

void order_moves(MoveList& move_List);
int quiescence(SearchState& vars, int alpha, int beta);
int search(SearchState& vars, int depth, int alpha, int beta);
void reset_search();
void run_search(SearchParams params);

#endif