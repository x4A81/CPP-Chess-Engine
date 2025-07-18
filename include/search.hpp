#ifndef SEARCH_HPP_INCLUDE
#define SEARCH_HPP_INCLUDE

#include <array>
#include <atomic>
#include <chrono>
#include <cassert>
#include <condition_variable>

#include "globals.hpp"

#define INF 10000
#define MATE_VALUE 9000
#define MAX_DEPTH 20
#define PV_TABLE_SIZE (max_ply*max_ply+max_ply)/2

#define FUTILITY_MARGIN 125 // 5/4 of a pawn

inline std::atomic<bool> stop_flag;

inline std::array<std::array<Move, 2>, max_ply> killer_moves = {{ nullmove }};
inline std::array<std::array<std::array<int, 2>, 64>, 64> history_moves {};
inline long nodes = 0;
inline std::chrono::steady_clock::time_point start_time;
inline std::array<Move, PV_TABLE_SIZE> prev_pv_table = { nullmove };
inline std::array<Move, PV_TABLE_SIZE> iid_pv_table = { nullmove };
inline Move fallback = nullmove;

/// @brief PV table helper. See https://www.chessprogramming.org/Triangular_PV-Table#Index.
/// @param ply Ply of pv.
/// @return The index to the pv table for ply.
inline int get_pv_index(int ply) { assert(ply < max_ply); return (ply*(2*max_ply+1-ply))/2; }

/// @brief PV table helper. See https://www.chessprogramming.org/Triangular_PV-Table#Index.
/// @param ply Ply of pv.
/// @return The index to the pv table for ply + 1.
inline int get_next_pv_index(int ply) { return get_pv_index(ply) + max_ply - ply; }

/// @brief Timer helper.
/// @param start_time Time to compare against.
/// @return The elasped time from start_time.
inline int elapsed_ms(std::chrono::steady_clock::time_point start_time) {
    using namespace std::chrono;
    return static_cast<int>(duration_cast<milliseconds>(
        steady_clock::now() - start_time
    ).count());
}

/*
These two functions were taken from this discussion:
https://talkchess.com/viewtopic.php?t=74411
*/

inline Score score_to_tt(Score score, int ply) { 
    return (score > MATE_VALUE) ? score + ply : (score < MATE_VALUE) ? score - ply : score; 
}

inline Score score_from_tt(Score score, int ply) {
    return (score > MATE_VALUE) ? score - ply : (score < MATE_VALUE) ? score + ply : score; 
}

#endif