#ifndef SEARCH_HPP_INCLUDE
#define SEARCH_HPP_INCLUDE

#include "board.hpp"
#include "transposition.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <cassert>

#define INF 10000
#define SEARCH_HELPER_THREADS 8

inline std::atomic<bool> stop_flag;

inline std::array<std::array<Move, 2>, MAX_PLY> killer_moves = {{ nullmove }};
inline std::array<std::array<std::array<int, 2>, 64>, 64> history_moves {};
inline std::array<Move, PV_TABLE_SIZE> prev_pv_table { nullmove };


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

#endif