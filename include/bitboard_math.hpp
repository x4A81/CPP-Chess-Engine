#ifndef BITBOARD_MATH_HPP_INCLUDE
#define BITBOARD_MATH_HPP_INCLUDE

#include <array>
#include <bit>
#include <cassert>

#include "globals.hpp"

namespace bb_math {

    // Board constants
    inline constexpr BB AFILE = 0x0101010101010101;
    inline constexpr BB HFILE = 0x8080808080808080;
    inline constexpr BB ABFILE = 0x0303030303030303;
    inline constexpr BB GHFILE = 0xC0C0C0C0C0C0C0C0;

    inline constexpr BB nAFILE = ~AFILE;
    inline constexpr BB nHFILE = ~HFILE;
    inline constexpr BB nABFILE = ~ABFILE;
    inline constexpr BB nGHFILE = ~GHFILE;

    // Pre-computed directional fills
    // anti-diagonal is north-west to south-east

    alignas(64) inline constexpr std::array<BB, 64> precomp_nort_fill = []() constexpr {
        std::array<BB, 64> arr{};
        BB gen;
        for (Square sq = 0; sq < 64; ++sq) {
            gen = BB(1) << sq;
            gen |= gen << 8;
            gen |= gen << 16;
            gen |= gen << 32;
            arr[sq] = gen;
        }

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_sout_fill = []() constexpr {
        std::array<BB, 64> arr{};
        BB gen;
        for (Square sq = 0; sq < 64; ++sq) {
            gen = BB(1) << sq;
            gen |= gen >> 8;
            gen |= gen >> 16;
            gen |= gen >> 32;
            arr[sq] = gen;
        }

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_east_fill = []() constexpr {
        std::array<BB, 64> arr{};
        BB gen;
        for (Square sq = 0; sq < 64; ++sq) {
            gen = BB(1) << sq;
            const BB pr0 = nAFILE;
            const BB pr1 = pr0 & (pr0 << 1);
            const BB pr2 = pr1 & (pr1 << 2);
            gen |= pr0 & (gen << 1);
            gen |= pr1 & (gen << 2);
            gen |= pr2 & (gen << 4);
            arr[sq] = gen;
        }

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_west_fill = []() constexpr {
        std::array<BB, 64> arr{};
        BB gen;
        for (Square sq = 0; sq < 64; ++sq) {
            gen = BB(1) << sq;
            const BB pr0 = nHFILE;
            const BB pr1 = pr0 & (pr0 >> 1);
            const BB pr2 = pr1 & (pr1 >> 2);
            gen |= pr0 & (gen >> 1); 
            gen |= pr1 & (gen >> 2);
            gen |= pr2 & (gen >> 4);
            arr[sq] = gen;
        }

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_noEast_fill = []() constexpr {
        std::array<BB, 64> arr{};
        BB gen;
        for (Square sq = 0; sq < 64; ++sq) {
            gen = BB(1) << sq;
            const BB pr0 = nAFILE;
            const BB pr1 = pr0 & (pr0 << 9);
            const BB pr2 = pr1 & (pr1 << 18);
            gen |= pr0 & (gen << 9);
            gen |= pr1 & (gen << 18);
            gen |= pr2 & (gen << 36);
            arr[sq] = gen;
        }

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_noWest_fill = []() constexpr {
        std::array<BB, 64> arr{};
        BB gen;
        for (Square sq = 0; sq < 64; ++sq) {
            gen = BB(1) << sq;
            const BB pr0 = nHFILE;
            const BB pr1 = pr0 & (pr0 << 7);
            const BB pr2 = pr1 & (pr1 << 14);
            gen |= pr0 & (gen << 7); 
            gen |= pr1 & (gen << 14);
            gen |= pr2 & (gen << 28);
            arr[sq] = gen;
        }

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_soEast_fill = []() constexpr {
        std::array<BB, 64> arr{};
        BB gen;
        for (Square sq = 0; sq < 64; ++sq) {
            gen = BB(1) << sq;
            const BB pr0 = nAFILE;
            const BB pr1 = pr0 & (pr0 >> 7);
            const BB pr2 = pr1 & (pr1 >> 14);
            gen |= pr0 & (gen >> 7); 
            gen |= pr1 & (gen >> 14);
            gen |= pr2 & (gen >> 28);
            arr[sq] = gen;
        }

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_soWest_fill = []() constexpr {
        std::array<BB, 64> arr{};
        BB gen;
        for (Square sq = 0; sq < 64; ++sq) {
            gen = BB(1) << sq;
            const BB pr0 = nHFILE;
            const BB pr1 = pr0 & (pr0 >> 9);
            const BB pr2 = pr1 & (pr1 >> 18);
            gen |= pr0 & (gen >> 9); 
            gen |= pr1 & (gen >> 18);
            gen |= pr2 & (gen >> 36);
            arr[sq] = gen;
        }

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_hor_fill = []() constexpr {
        std::array<BB, 64> arr{};
        for (Square sq = 0; sq < 64; ++sq)
            arr[sq] = precomp_east_fill[sq] | precomp_west_fill[sq];

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_ver_fill = []() constexpr {
        std::array<BB, 64> arr{};
        for (Square sq = 0; sq < 64; ++sq)
            arr[sq] = precomp_nort_fill[sq] | precomp_sout_fill[sq];

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_dia_fill = []() constexpr {
        std::array<BB, 64> arr{};
        for (Square sq = 0; sq < 64; ++sq)
            arr[sq] = precomp_noEast_fill[sq] | precomp_soWest_fill[sq];

        return arr;
    }();

    alignas(64) inline constexpr std::array<BB, 64> precomp_antdia_fill = []() constexpr {
        std::array<BB, 64> arr{};
        for (Square sq = 0; sq < 64; ++sq)
            arr[sq] = precomp_noWest_fill[sq] | precomp_soEast_fill[sq];

        return arr;
    }();
    
    enum Dir : int {
        nort, sout, east, west,
        noEast, noWest,
        soEast, soWest,
        noNoEa, noEaEa,
        soEaEa, soSoEa,
        noNoWe, noWeWe,
        soWeWe, soSoWe,
    };
    
    /// @brief Used to avoid wraps when rotating bitboards, indexed by Dir
    alignas(64) inline constexpr std::array<BB, 8> avoid_wraps = { 
        0xFFFFFFFFFFFFFF00, 0x00FFFFFFFFFFFFFF, 0xFEFEFEFEFEFEFEFE, 0x7F7F7F7F7F7F7F7F, 
        0xFEFEFEFEFEFEFE00, 0x7F7F7F7F7F7F7F00, 0x00FEFEFEFEFEFEFE, 0x007F7F7F7F7F7F7F 
    };
    
    /// @brief Used to shift bitboards in the direction of Dir
    inline constexpr std::array<int, 16> shifts = { 8,  -8,  1,  -1,  9, 7,  -7,  -9,
                                         17, 10, -6, -15, 15, 6, -10, -17 };
    
    /// @brief Square masking helper.
    /// @param sq Square to mask.
    /// @return Bitboard of masked square.
    [[gnu::always_inline, gnu::hot]]
    inline constexpr BB mask(Square sq) { 
        assert(sq >= 0 && sq < 64); return BB(1) << sq; }

    /// @brief Bit popper helper.
    /// @param bb Reference to bitboard.
    /// @param sq Square index in bitboard to pop.
    [[gnu::always_inline, gnu::hot]]
    inline void pop_bit(BB& bb, Square sq) { bb &= ~mask(sq); }

    /// @brief Bit setter helper.
    /// @param bb Reference to bitboard.
    /// @param sq Square index in bitboard to pop.
    [[gnu::always_inline, gnu::hot]]
    inline void set_bit(BB& bb, Square sq) { bb |= mask(sq); }

    /// @brief Bit getter helper.
    /// @param bb Reference to bitboard.
    /// @param sq Square index in bitboard to get.
    /// @return 1 if bit is set, 0 if not set.
    [[gnu::always_inline, gnu::hot]]
    inline bool get_bit(BB bb, Square sq) { return bb & (BB(1) << sq); }

    /// @brief Population counter helper.
    /// @param bb Bitboard to count.
    /// @return The number of set bits in bb.
    [[gnu::always_inline, gnu::hot]]
    inline int pop_count(BB bb) { return std::popcount(bb); }
    
    /// @brief Bit scanner helper.
    /// @param bb Bitboard to scan.
    /// @return The index of the least significant set bit in bb.
    [[gnu::always_inline, gnu::hot]]
    inline int bitscan_forward(BB bb) { assert(bb != 0); return std::countr_zero(bb); }

    /// @brief Bit popper helper. Pops the least significant set bit and returns its index.
    /// @param bb Reference to bitboard.
    /// @return The index of the least significant set bit.
    [[gnu::always_inline, gnu::hot]]
    inline int pop_lsb(BB& bb) {
        assert(bb != 0);
        int sq = std::countr_zero(bb);
        bb &= bb - 1;
        return sq;
    }
    
    /// @brief Generalised bit shifter. See https://www.chessprogramming.org/General_Setwise_Operations#Generalized_Shift.
    /// @param bb Bitboard to shift.
    /// @param dir Direction to shift.
    /// @return The shifted bitboard.
    [[gnu::always_inline, gnu::hot]]
    inline BB shift_one(BB bb, Dir dir) {
        int s = shifts[dir];
        return std::rotl(bb, s) & avoid_wraps[dir];
    }

    /// @brief Generalised fill with blockers helper. See https://www.chessprogramming.org/Kogge-Stone_Algorithm#Generalized_Rays.
    /// @param bb Bitboard to fill.
    /// @param occ Bitboard of blockers.
    /// @param dir Direction to fill.
    /// @return Filled bitboard.
    [[gnu::always_inline, gnu::hot]]
    inline BB occ_fill(BB bb, BB occ, Dir dir) {
        int s = shifts[dir];
        occ &= avoid_wraps[dir];
        bb |= occ & std::rotl(bb, s);
        occ &= std::rotl(occ, s);
        bb |= occ & std::rotl(bb, s*2);
        occ &= std::rotl(occ, s*2);
        bb |= occ & std::rotl(bb, s*3);
        return bb;
    }

    /// @brief Sliding attacks helper. See https://www.chessprogramming.org/Kogge-Stone_Algorithm#Generalized_Rays.
    /// @param sliders Bitboard of sliders.
    /// @param occ Bitboard of blockers.
    /// @param dir Direction of moves.
    /// @return Bitboard of sliding attacks.
    [[gnu::always_inline, gnu::hot]]
    inline BB sliding_attacks(BB sliders, BB occ, Dir dir) {
        return shift_one(occ_fill(sliders, ~occ, dir), dir);
    }

    /// @brief Fill helper.
    /// @param gen Starting Bitboard.
    /// @return Bitboard of all bits smeared north.
    inline BB north_fill(BB gen) {
        gen |= (gen << 8);
        gen |= (gen << 16);
        gen |= (gen << 32);
        return gen;
    }

    /// @brief Fill helper.
    /// @param gen Starting Bitboard.
    /// @return Bitboard of all bits smeared south.
    inline BB south_fill(BB gen) {
        gen |= (gen >> 8);
        gen |= (gen >> 16);
        gen |= (gen >> 32);
        return gen;
    }

    /// @brief Fill helper.
    /// @param gen Starting Bitboard.
    /// @return Bitboard of all bits smeared north and south.
    inline BB file_fill(BB gen) { return north_fill(gen) | south_fill(gen); }

    inline BB wfront_span(BB wpawns) { return north_fill(wpawns) << 8; }
    inline BB brear_span(BB bpawns) { return north_fill(bpawns) << 8; }
    inline BB bfront_span(BB bpawns) { return south_fill(bpawns) >> 8; }
    inline BB wrear_span(BB wpawns) { return south_fill(wpawns) >> 8; }

    inline BB wpassed_pawns(BB wpawns, BB bpawns) {
        BB allFrontSpans = bfront_span(bpawns);
        allFrontSpans |= shift_one(allFrontSpans, east)
                        |  shift_one(allFrontSpans, west);
        return wpawns & ~allFrontSpans;
    }

    inline BB bpassed_pawns(BB bpawns, BB wpawns) {
        BB allFrontSpans = wfront_span(wpawns);
        allFrontSpans |= shift_one(allFrontSpans, east)
                        |  shift_one(allFrontSpans, west);
        return bpawns & ~allFrontSpans;
    }

    inline BB no_neighbor_east_file(BB pawns) {
        return pawns & ~shift_one(file_fill(pawns), west);
    }

    inline BB no_neighbor_west_file(BB pawns) {
        return pawns & ~shift_one(file_fill(pawns), east);
    }

    inline BB isolanis(BB pawns) {
    return  no_neighbor_east_file(pawns)
            & no_neighbor_west_file(pawns);
    }

    inline BB half_isolanis(BB pawns) {
    return  no_neighbor_east_file(pawns)
            ^ no_neighbor_west_file(pawns);
    }

    inline BB open_file(BB wpanws, BB bpawns) {
    return ~file_fill(wpanws) & ~file_fill(bpawns);
    }

    inline BB half_open_or_open(BB gen) { return ~file_fill(gen); }

    inline BB w_half_open_files(BB wpawns, BB bpawns) {
    return half_open_or_open(wpawns)
            ^ open_file(wpawns, bpawns);
    }

    inline BB b_half_open_files(BB wpawns, BB bpawns) {
    return half_open_or_open(bpawns)
            ^ open_file(wpawns, bpawns);
    }
}

#endif