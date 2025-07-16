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

    alignas(64) inline constexpr std::array<BB, 64> precomp_hor_fill = {
        0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f,     
        0xfe00, 0xfd00, 0xfb00, 0xf700, 0xef00, 0xdf00, 0xbf00, 0x7f00, 
        0xfe0000, 0xfd0000, 0xfb0000, 0xf70000, 0xef0000, 0xdf0000, 0xbf0000, 0x7f0000, 
        0xfe000000, 0xfd000000, 0xfb000000, 0xf7000000, 0xef000000, 0xdf000000, 0xbf000000, 0x7f000000, 
        0xfe00000000, 0xfd00000000, 0xfb00000000, 0xf700000000, 0xef00000000, 0xdf00000000, 0xbf00000000, 0x7f00000000, 
        0xfe0000000000, 0xfd0000000000, 0xfb0000000000, 0xf70000000000, 0xef0000000000, 0xdf0000000000, 0xbf0000000000, 0x7f0000000000, 
        0xfe000000000000, 0xfd000000000000, 0xfb000000000000, 0xf7000000000000, 0xef000000000000, 0xdf000000000000, 0xbf000000000000, 0x7f000000000000, 
        0xfe00000000000000, 0xfd00000000000000, 0xfb00000000000000, 0xf700000000000000, 0xef00000000000000, 0xdf00000000000000, 0xbf00000000000000, 0x7f00000000000000
    };

    alignas(64) inline constexpr std::array<BB, 64> precomp_ver_fill = {
        0x101010101010100, 0x202020202020200, 0x404040404040400, 0x808080808080800, 0x1010101010101000, 0x2020202020202000, 0x4040404040404000, 0x8080808080808000, 
        0x101010101010001, 0x202020202020002, 0x404040404040004, 0x808080808080008, 0x1010101010100010, 0x2020202020200020, 0x4040404040400040, 0x8080808080800080, 
        0x101010101000101, 0x202020202000202, 0x404040404000404, 0x808080808000808, 0x1010101010001010, 0x2020202020002020, 0x4040404040004040, 0x8080808080008080, 
        0x101010100010101, 0x202020200020202, 0x404040400040404, 0x808080800080808, 0x1010101000101010, 0x2020202000202020, 0x4040404000404040, 0x8080808000808080, 
        0x101010001010101, 0x202020002020202, 0x404040004040404, 0x808080008080808, 0x1010100010101010, 0x2020200020202020, 0x4040400040404040, 0x8080800080808080, 
        0x101000101010101, 0x202000202020202, 0x404000404040404, 0x808000808080808, 0x1010001010101010, 0x2020002020202020, 0x4040004040404040, 0x8080008080808080, 
        0x100010101010101, 0x200020202020202, 0x400040404040404, 0x800080808080808, 0x1000101010101010, 0x2000202020202020, 0x4000404040404040, 0x8000808080808080, 
        0x1010101010101, 0x2020202020202, 0x4040404040404, 0x8080808080808, 0x10101010101010, 0x20202020202020, 0x40404040404040, 0x80808080808080
    };

    alignas(64) inline constexpr std::array<BB, 64> precomp_dia_fill = {
        0x8040201008040200, 0x80402010080400, 0x804020100800, 0x8040201000, 0x80402000, 0x804000, 0x8000, 0x0, 
        0x4020100804020000, 0x8040201008040001, 0x80402010080002, 0x804020100004, 0x8040200008, 0x80400010, 0x800020, 0x40, 
        0x2010080402000000, 0x4020100804000100, 0x8040201008000201, 0x80402010000402, 0x804020000804, 0x8040001008, 0x80002010, 0x4020, 
        0x1008040200000000, 0x2010080400010000, 0x4020100800020100, 0x8040201000040201, 0x80402000080402, 0x804000100804, 0x8000201008, 0x402010, 
        0x804020000000000, 0x1008040001000000, 0x2010080002010000, 0x4020100004020100, 0x8040200008040201, 0x80400010080402, 0x800020100804, 0x40201008, 
        0x402000000000000, 0x804000100000000, 0x1008000201000000, 0x2010000402010000, 0x4020000804020100, 0x8040001008040201, 0x80002010080402, 0x4020100804, 
        0x200000000000000, 0x400010000000000, 0x800020100000000, 0x1000040201000000, 0x2000080402010000, 0x4000100804020100, 0x8000201008040201, 0x402010080402, 
        0x0, 0x1000000000000, 0x2010000000000, 0x4020100000000, 0x8040201000000, 0x10080402010000, 0x20100804020100, 0x40201008040201
    };

    alignas(64) inline constexpr std::array<BB, 64> precomp_antdia_fill = {
        0x0, 0x100, 0x10200, 0x1020400, 0x102040800, 0x10204081000, 0x1020408102000, 0x102040810204000, 
        0x2, 0x10004, 0x1020008, 0x102040010, 0x10204080020, 0x1020408100040, 0x102040810200080, 0x204081020400000, 
        0x204, 0x1000408, 0x102000810, 0x10204001020, 0x1020408002040, 0x102040810004080, 0x204081020008000, 0x408102040000000, 
        0x20408, 0x100040810, 0x10200081020, 0x1020400102040, 0x102040800204080, 0x204081000408000, 0x408102000800000, 0x810204000000000, 
        0x2040810, 0x10004081020, 0x1020008102040, 0x102040010204080, 0x204080020408000, 0x408100040800000, 0x810200080000000, 0x1020400000000000, 
        0x204081020, 0x1000408102040, 0x102000810204080, 0x204001020408000, 0x408002040800000, 0x810004080000000, 0x1020008000000000, 0x2040000000000000, 
        0x20408102040, 0x100040810204080, 0x200081020408000, 0x400102040800000, 0x800204080000000, 0x1000408000000000, 0x2000800000000000, 0x4000000000000000, 
        0x2040810204080, 0x4081020408000, 0x8102040800000, 0x10204080000000, 0x20408000000000, 0x40800000000000, 0x80000000000000, 0x0
    };
    
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
    inline int get_bit(BB bb, Square sq) { return (bb >> sq) & 1; }

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
    /// @param sq Square to fill on.
    /// @return Bitboard of horizontal fill.
    [[gnu::always_inline, gnu::hot]]
    inline constexpr BB hor_fill(Square sq) {
        return precomp_hor_fill[sq];
    }

    /// @brief Fill helper.
    /// @param sq Square to fill on.
    /// @return Bitboard of vertical fill.
    [[gnu::always_inline, gnu::hot]]
    inline BB ver_fill(Square sq) {
        return precomp_ver_fill[sq];
    }

    /// @brief Fill helper.
    /// @param sq Square to fill on.
    /// @return Bitboard of diagonal fill.
    [[gnu::always_inline, gnu::hot]]
    inline BB dia_fill(Square sq) {
        return precomp_dia_fill[sq];
    }

    /// @brief Fill helper.
    /// @param sq Square to fill on.
    /// @return Bitboard of anti-diagonal fill.
    [[gnu::always_inline, gnu::hot]]
    inline BB antdia_fill(Square sq) {
        return precomp_antdia_fill[sq];
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