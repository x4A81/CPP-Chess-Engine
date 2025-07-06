#ifndef SEARCH_HPP_INCLUDE
#define SEARCH_HPP_INCLUDE

#include "board.hpp"
#include "transposition.hpp"
#include <array>
#include <atomic>
#include <chrono>

#define INF 100000
#define NOT_USED -1;

inline std::atomic<bool> stop_flag;
inline Transposition transposition_table;

struct SearchParams {
    int max_depth = 0;
    int nodes = NOT_USED;
    int move_time = NOT_USED;
    int inc = NOT_USED;
    int total_time = NOT_USED;
    bool infinite = false;

    // Unsupported
    // bool ponder = false;
};

inline int get_pv_index(int ply) { return (ply*(2*MAX_PLY+1-ply))/2; }
inline int get_next_pv_index(int ply) { return get_pv_index(ply) + MAX_PLY - ply; }
inline int elapsed_ms(std::chrono::steady_clock::time_point time) {
    using namespace std::chrono;
    return static_cast<int>(duration_cast<milliseconds>(
        steady_clock::now() - time
    ).count());
}

#endif