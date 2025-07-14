#ifndef SEARCH_HPP_INCLUDE
#define SEARCH_HPP_INCLUDE

#include "board.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <cassert>
#include <mutex>
#include <condition_variable>

using namespace std;

#define INF 10000
#define MATE_VALUE 9000
#define MAX_DEPTH 20
#define MAX_PLY 64
#define PV_TABLE_SIZE (MAX_PLY*MAX_PLY+MAX_PLY)/2

#define FUTILITY_MARGIN 125 // 5/4 of a pawn

inline atomic<bool> stop_flag;

inline array<array<Move, 2>, MAX_PLY> killer_moves = {{ nullmove }};
inline array<array<array<int, 2>, 64>, 64> history_moves {};
inline long nodes = 0;
inline chrono::steady_clock::time_point start_time;
inline array<Move, PV_TABLE_SIZE> prev_pv_table = { nullmove };

/// @brief PV table helper. See https://www.chessprogramming.org/Triangular_PV-Table#Index.
/// @param ply Ply of pv.
/// @return The index to the pv table for ply.
inline int get_pv_index(int ply) { assert(ply < MAX_PLY); return (ply*(2*MAX_PLY+1-ply))/2; }

/// @brief PV table helper. See https://www.chessprogramming.org/Triangular_PV-Table#Index.
/// @param ply Ply of pv.
/// @return The index to the pv table for ply + 1.
inline int get_next_pv_index(int ply) { return get_pv_index(ply) + MAX_PLY - ply; }

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