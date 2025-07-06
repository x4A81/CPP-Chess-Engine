#ifndef BITBOARD_UTILS_HPP_INCLUDE
#define BITBOARD_UTILS_HPP_INCLUDE

using namespace std;

#include <cstdint>
#include <array>
#include <bit>

using BB = uint64_t;

namespace bitboard_utils {

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

    alignas(64) inline constexpr array<BB, 64> precomp_hor_fill = {
        0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f,     
        0xfe00, 0xfd00, 0xfb00, 0xf700, 0xef00, 0xdf00, 0xbf00, 0x7f00, 
        0xfe0000, 0xfd0000, 0xfb0000, 0xf70000, 0xef0000, 0xdf0000, 0xbf0000, 0x7f0000, 
        0xfe000000, 0xfd000000, 0xfb000000, 0xf7000000, 0xef000000, 0xdf000000, 0xbf000000, 0x7f000000, 
        0xfe00000000, 0xfd00000000, 0xfb00000000, 0xf700000000, 0xef00000000, 0xdf00000000, 0xbf00000000, 0x7f00000000, 
        0xfe0000000000, 0xfd0000000000, 0xfb0000000000, 0xf70000000000, 0xef0000000000, 0xdf0000000000, 0xbf0000000000, 0x7f0000000000, 
        0xfe000000000000, 0xfd000000000000, 0xfb000000000000, 0xf7000000000000, 0xef000000000000, 0xdf000000000000, 0xbf000000000000, 0x7f000000000000, 
        0xfe00000000000000, 0xfd00000000000000, 0xfb00000000000000, 0xf700000000000000, 0xef00000000000000, 0xdf00000000000000, 0xbf00000000000000, 0x7f00000000000000
    };

    alignas(64) inline constexpr array<BB, 64> precomp_ver_fill = {
        0x101010101010100, 0x202020202020200, 0x404040404040400, 0x808080808080800, 0x1010101010101000, 0x2020202020202000, 0x4040404040404000, 0x8080808080808000, 
        0x101010101010001, 0x202020202020002, 0x404040404040004, 0x808080808080008, 0x1010101010100010, 0x2020202020200020, 0x4040404040400040, 0x8080808080800080, 
        0x101010101000101, 0x202020202000202, 0x404040404000404, 0x808080808000808, 0x1010101010001010, 0x2020202020002020, 0x4040404040004040, 0x8080808080008080, 
        0x101010100010101, 0x202020200020202, 0x404040400040404, 0x808080800080808, 0x1010101000101010, 0x2020202000202020, 0x4040404000404040, 0x8080808000808080, 
        0x101010001010101, 0x202020002020202, 0x404040004040404, 0x808080008080808, 0x1010100010101010, 0x2020200020202020, 0x4040400040404040, 0x8080800080808080, 
        0x101000101010101, 0x202000202020202, 0x404000404040404, 0x808000808080808, 0x1010001010101010, 0x2020002020202020, 0x4040004040404040, 0x8080008080808080, 
        0x100010101010101, 0x200020202020202, 0x400040404040404, 0x800080808080808, 0x1000101010101010, 0x2000202020202020, 0x4000404040404040, 0x8000808080808080, 
        0x1010101010101, 0x2020202020202, 0x4040404040404, 0x8080808080808, 0x10101010101010, 0x20202020202020, 0x40404040404040, 0x80808080808080
    };

    alignas(64) inline constexpr array<BB, 64> precomp_dia_fill = {
        0x8040201008040200, 0x80402010080400, 0x804020100800, 0x8040201000, 0x80402000, 0x804000, 0x8000, 0x0, 
        0x4020100804020000, 0x8040201008040001, 0x80402010080002, 0x804020100004, 0x8040200008, 0x80400010, 0x800020, 0x40, 
        0x2010080402000000, 0x4020100804000100, 0x8040201008000201, 0x80402010000402, 0x804020000804, 0x8040001008, 0x80002010, 0x4020, 
        0x1008040200000000, 0x2010080400010000, 0x4020100800020100, 0x8040201000040201, 0x80402000080402, 0x804000100804, 0x8000201008, 0x402010, 
        0x804020000000000, 0x1008040001000000, 0x2010080002010000, 0x4020100004020100, 0x8040200008040201, 0x80400010080402, 0x800020100804, 0x40201008, 
        0x402000000000000, 0x804000100000000, 0x1008000201000000, 0x2010000402010000, 0x4020000804020100, 0x8040001008040201, 0x80002010080402, 0x4020100804, 
        0x200000000000000, 0x400010000000000, 0x800020100000000, 0x1000040201000000, 0x2000080402010000, 0x4000100804020100, 0x8000201008040201, 0x402010080402, 
        0x0, 0x1000000000000, 0x2010000000000, 0x4020100000000, 0x8040201000000, 0x10080402010000, 0x20100804020100, 0x40201008040201
    };

    alignas(64) inline constexpr array<BB, 64> precomp_antdia_fill = {
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
        nort,
        sout,
        east,
        west,
        noEast,
        noWest,
        soEast,
        soWest,
        noNoEa,
        noEaEa,
        soEaEa,
        soSoEa,
        noNoWe,
        noWeWe,
        soWeWe,
        soSoWe,
    };
    
    /// @brief Used to avoid wraps when rotating bitboards, indexed by Dir
    alignas(64) inline constexpr std::array<BB, 8> avoid_wraps = { 
        0xFFFFFFFFFFFFFF00, 0x00FFFFFFFFFFFFFF, 0xFEFEFEFEFEFEFEFE, 0x7F7F7F7F7F7F7F7F, 
        0xFEFEFEFEFEFEFE00, 0x7F7F7F7F7F7F7F00, 0x00FEFEFEFEFEFEFE, 0x007F7F7F7F7F7F7F 
    };
    
    /// @brief Used to shift bitboards in the direction of Dir
    inline constexpr std::array<int, 16> shifts = { 8,  -8,  1,  -1,  9, 7,  -7,  -9,
                                         17, 10, -6, -15, 15, 6, -10, -17 };
    
    /// @brief Mask the square to a bitboard
    inline constexpr BB mask(int sq) { return BB(1) << sq; }

    /// @brief Pop the bit at square in the bitboard
    inline void pop_bit(BB& bb, int sq) { bb &= ~mask(sq); }

    /// @brief Set the bit at square in the bitboard
    inline void set_bit(BB& bb, int sq) { bb |= mask(sq); }

    /// @brief Get the bit at square in the bitboard
    inline int get_bit(BB bb, int sq) { return (bb >> sq) & 1; }

    /// @brief Find the index of the least significant bit set in the bitboard
    inline int bitscan_forward(BB bb) { return countr_zero(bb); }

    /// @brief Count the number of bits set in the bitboard
    inline int pop_count(BB bb) { return popcount(bb); }
    
    /// @brief Find the index of the most significan set bit
    inline int bitscan_reverse(BB bb) { return 63 - countl_zero(bb); }

    /// @brief Pop the least significant bit from the bitboard and return its index
    inline int pop_lsb(BB& bb) {
        int sq = countr_zero(bb);
        bb &= bb - 1;
        return sq;
    }
    
    /// @brief Generalised shift for bitboards
    inline BB shift_one(BB bb, Dir dir) {
        int s = shifts[dir];
        return rotl(bb, s) & avoid_wraps[dir];
    }

    /// @brief Generalised fill for sliding pieces, stopping at blockers
    inline BB occ_fill(BB gen, BB pro, Dir dir) {
        int s = shifts[dir];
        pro &= avoid_wraps[dir];
        gen |= pro & rotl(gen, s);
        pro &= rotl(pro, s);
        gen |= pro & rotl(gen, s*2);
        pro &= rotl(pro, s*2);
        gen |= pro & rotl(gen, s*3);
        return gen;
    }

    /// @brief Generalised sliding attacks, given a set of sliders and occupied squares
    inline BB sliding_attacks(BB sliders, BB occ, Dir dir) {
        return shift_one(occ_fill(sliders, ~occ, dir), dir);
    }

    /// @brief Fill horizontal
    inline constexpr BB hor_fill(int sq) {
        return precomp_hor_fill[sq];
    }

    /// @brief Fill vertical
    inline BB ver_fill(int sq) {
        return precomp_ver_fill[sq];
    }

    /// @brief Fill diagonal
    inline BB dia_fill(int sq) {
        return precomp_dia_fill[sq];
    }

    /// @brief Fill antidiagonal
    inline BB antdia_fill(int sq) {
        return precomp_antdia_fill[sq];
    }
}

namespace move_generator {
    using namespace bitboard_utils;

    /// @brief Get white pawn attacks
    inline BB wpawn_attacks(BB pawns) {
        return ((pawns << 9) & nAFILE) | (pawns << 7) & nHFILE;
    }

    /// @brief Get black pawn attacks
    inline BB bpawn_attacks(BB pawns) {
        return ((pawns >> 9) & nHFILE) | ((pawns >> 7) & nAFILE);
    }

    /// @brief Get knight attacks
    inline BB knight_attacks(BB knights) {
        BB l1 = (knights >> 1) & 0x7f7f7f7f7f7f7f7f;
        BB l2 = (knights >> 2) & 0x3f3f3f3f3f3f3f3f;
        BB r1 = (knights << 1) & 0xfefefefefefefefe;
        BB r2 = (knights << 2) & 0xfcfcfcfcfcfcfcfc;
        BB h1 = l1 | r1;
        BB h2 = l2 | r2;
        return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
    }

    /// @brief Pawn attack table, indexed by [sq][side]
    alignas(64) inline constexpr array<array<BB, 2>, 64> pawn_attack_table = {{
        {{ 0x0, 0x200 }}, {{ 0x0, 0x500 }}, {{ 0x0, 0xa00 }}, {{ 0x0, 0x1400 }},
        {{ 0x0, 0x2800 }}, {{ 0x0, 0x5000 }}, {{ 0x0, 0xa000 }}, {{ 0x0, 0x4000 }},
        {{ 0x2, 0x20000 }}, {{ 0x5, 0x50000 }}, {{ 0xa, 0xa0000 }}, {{ 0x14, 0x140000 }},
        {{ 0x28, 0x280000 }}, {{ 0x50, 0x500000 }}, {{ 0xa0, 0xa00000 }}, {{ 0x40, 0x400000 }},
        {{ 0x200, 0x2000000 }}, {{ 0x500, 0x5000000 }}, {{ 0xa00, 0xa000000 }}, {{ 0x1400, 0x14000000 }},
        {{ 0x2800, 0x28000000 }}, {{ 0x5000, 0x50000000 }}, {{ 0xa000, 0xa0000000 }}, {{ 0x4000, 0x40000000 }},
        {{ 0x20000, 0x200000000 }}, {{ 0x50000, 0x500000000 }}, {{ 0xa0000, 0xa00000000 }}, {{ 0x140000, 0x1400000000 }},
        {{ 0x280000, 0x2800000000 }}, {{ 0x500000, 0x5000000000 }}, {{ 0xa00000, 0xa000000000 }}, {{ 0x400000, 0x4000000000 }},
        {{ 0x2000000, 0x20000000000 }}, {{ 0x5000000, 0x50000000000 }}, {{ 0xa000000, 0xa0000000000 }}, {{ 0x14000000, 0x140000000000 }},
        {{ 0x28000000, 0x280000000000 }}, {{ 0x50000000, 0x500000000000 }}, {{ 0xa0000000, 0xa00000000000 }}, {{ 0x40000000, 0x400000000000 }},
        {{ 0x200000000, 0x2000000000000 }}, {{ 0x500000000, 0x5000000000000 }}, {{ 0xa00000000, 0xa000000000000 }}, {{ 0x1400000000, 0x14000000000000 }},
        {{ 0x2800000000, 0x28000000000000 }}, {{ 0x5000000000, 0x50000000000000 }}, {{ 0xa000000000, 0xa0000000000000 }}, {{ 0x4000000000, 0x40000000000000 }},
        {{ 0x20000000000, 0x200000000000000 }}, {{ 0x50000000000, 0x500000000000000 }}, {{ 0xa0000000000, 0xa00000000000000 }}, {{ 0x140000000000, 0x1400000000000000 }},
        {{ 0x280000000000, 0x2800000000000000 }}, {{ 0x500000000000, 0x5000000000000000 }}, {{ 0xa00000000000, 0xa000000000000000 }}, {{ 0x400000000000, 0x4000000000000000 }},
        {{ 0x2000000000000, 0x0 }}, {{ 0x5000000000000, 0x0 }}, {{ 0xa000000000000, 0x0 }}, {{ 0x14000000000000, 0x0 }},
        {{ 0x28000000000000, 0x0 }}, {{ 0x50000000000000, 0x0 }}, {{ 0xa0000000000000, 0x0 }}, {{ 0x40000000000000, 0x0 }}
    }};

    /// @brief Knight attack table, indexed by square
    alignas(64) inline constexpr array<BB, 64> knight_move_table = { 0x20400, 0x50800, 0xa1100, 0x142200, 0x284400, 0x508800, 0xa01000, 0x402000, 0x2040004, 0x5080008, 0xa110011, 0x14220022, 0x28440044, 0x50880088, 0xa0100010, 0x40200020, 0x204000402, 0x508000805, 0xa1100110a, 0x1422002214, 0x2844004428, 0x5088008850, 0xa0100010a0, 0x4020002040, 0x20400040200, 0x50800080500, 0xa1100110a00, 0x142200221400, 0x284400442800, 0x508800885000, 0xa0100010a000, 0x402000204000, 0x2040004020000, 0x5080008050000, 0xa1100110a0000, 0x14220022140000, 0x28440044280000, 0x50880088500000, 0xa0100010a00000, 0x40200020400000, 0x204000402000000, 0x508000805000000, 0xa1100110a000000, 0x1422002214000000, 0x2844004428000000, 0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000, 0x400040200000000, 0x800080500000000, 0x1100110a00000000, 0x2200221400000000, 0x4400442800000000, 0x8800885000000000, 0x100010a000000000, 0x2000204000000000, 0x4020000000000, 0x8050000000000, 0x110a0000000000, 0x22140000000000, 0x44280000000000, 0x88500000000000, 0x10a00000000000, 0x20400000000000 };
    alignas(64) inline constexpr array<BB, 64> king_move_table = { 0x302, 0x705, 0xe0a, 0x1c14, 0x3828, 0x7050, 0xe0a0, 0xc040, 0x30203, 0x70507, 0xe0a0e, 0x1c141c, 0x382838, 0x705070, 0xe0a0e0, 0xc040c0, 0x3020300, 0x7050700, 0xe0a0e00, 0x1c141c00, 0x38283800, 0x70507000, 0xe0a0e000, 0xc040c000, 0x302030000, 0x705070000, 0xe0a0e0000, 0x1c141c0000, 0x3828380000, 0x7050700000, 0xe0a0e00000, 0xc040c00000, 0x30203000000, 0x70507000000, 0xe0a0e000000, 0x1c141c000000, 0x382838000000, 0x705070000000, 0xe0a0e0000000, 0xc040c0000000, 0x3020300000000, 0x7050700000000, 0xe0a0e00000000, 0x1c141c00000000, 0x38283800000000, 0x70507000000000, 0xe0a0e000000000, 0xc040c000000000, 0x302030000000000, 0x705070000000000, 0xe0a0e0000000000, 0x1c141c0000000000, 0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000, 0x203000000000000, 0x507000000000000, 0xa0e000000000000, 0x141c000000000000, 0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000 };

    /// @brief Magics taken from my old code
    alignas(64) inline constexpr array<BB, 64> rook_magics = { 0x6080104000208000, 0x240400010002000, 0x8080200010008008, 0x4080100080080004, 0x2080080004008003, 0x880040080020001, 0x280020000800100, 0x100005021000082, 0x2000802080004000, 0x80200080400c, 0x801000200080, 0x1002008100100, 0x41801800040080, 0x1000400020900, 0x3000200010004, 0x1000080420100, 0x208000804008, 0x1010040002080, 0x10120040208200, 0x808010000800, 0x4000808004000800, 0x101010004000802, 0x4080808002000100, 0x100020000840041, 0x800802080004008, 0x2000500040002000, 0x1004100200010, 0x10008080100800, 0x1004008080080004, 0x20080800400, 0x100a100040200, 0x8200010044, 0x40800101002040, 0x10006000c00040, 0x100080802004, 0x400900089002100, 0x2000510005000800, 0x800400800200, 0x1800200800100, 0x11c0082000041, 0x4180002000404000, 0x800420081020024, 0x200010008080, 0x2000100100210008, 0x60040008008080, 0x4000201004040, 0x4080201040010, 0x4100820004, 0x10800040002080, 0x40002000804080, 0x8000200080100080, 0x8200810010100, 0x8000080080040080, 0x4008004020080, 0x1000010810028400, 0x140100488200, 0x40144100288001, 0x8000820021004012, 0x4100400c200011, 0x11000420081001, 0x802002144100882, 0x1000204000801, 0x8450002000400c1, 0x10004100802402 };
    alignas(64) inline constexpr array<BB, 64> bishop_magics = { 0x2088080100440100, 0x8124802410180, 0x4042082000000, 0x29040100c01800, 0x8004042000000004, 0x2080208200010, 0x100a820088000, 0x1008090011080, 0x200042004841480, 0x8020100101041080, 0x11060206120280, 0x10842402800400, 0x11045040000000, 0x2020008250400000, 0x1100004104104000, 0x2104100400, 0x440010408082126, 0x5081001081100, 0x10000800401822, 0x8000800802004010, 0x45000090400000, 0x4085202020200, 0x450202100409, 0x400022080400, 0x4200010021008, 0x2080010300080, 0x10900018004010, 0x8004040080410200, 0x9010000104000, 0x100410022010100, 0x2104004000880400, 0x908009042080, 0x10442010110298, 0x1280298881040, 0x140200040800, 0x400808108200, 0x8040024010030100, 0x8500020010083, 0x84040090005800, 0x404010040002402, 0x2025182010000420, 0x4014014950204800, 0x941088001000, 0x8000a02011002800, 0x1000080100400400, 0x210200a5008200, 0x10014104010100, 0x1014200802a00, 0x1080804840800, 0x220802080200, 0x10020201440800, 0x814084040400, 0x1202020004, 0x800088208020400, 0x2008101002005002, 0x4280200620000, 0x2010402510400, 0x8004202012000, 0x400000028841001, 0x200008000840428, 0x1001040050100, 0x504080201, 0x1020045110110109, 0x1020208102102040, };
    
    /// @brief Rook shifts for magics
    alignas(64) inline constexpr array<int, 64> rook_shifts = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
    };

    /// @brief Bishop shifts for magics
    alignas(64) inline constexpr array<int, 64> bishop_shifts = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
    };

    /// @brief Rook movement masks, used for sliding piece attacks
    alignas(64) inline constexpr array<BB, 64> rook_movement_masks = {
        0x000101010101017e, 0x000202020202027c, 0x000404040404047a, 0x0008080808080876, 
        0x001010101010106e, 0x002020202020205e, 0x004040404040403e, 0x008080808080807e, 
        0x0001010101017e00, 0x0002020202027c00, 0x0004040404047a00, 0x0008080808087600, 
        0x0010101010106e00, 0x0020202020205e00, 0x0040404040403e00, 0x0080808080807e00, 
        0x00010101017e0100, 0x00020202027c0200, 0x00040404047a0400, 0x0008080808760800, 
        0x00101010106e1000, 0x00202020205e2000, 0x00404040403e4000, 0x00808080807e8000, 
        0x000101017e010100, 0x000202027c020200, 0x000404047a040400, 0x0008080876080800, 
        0x001010106e101000, 0x002020205e202000, 0x004040403e404000, 0x008080807e808000, 
        0x0001017e01010100, 0x0002027c02020200, 0x0004047a04040400, 0x0008087608080800, 
        0x0010106e10101000, 0x0020205e20202000, 0x0040403e40404000, 0x0080807e80808000, 
        0x00017e0101010100, 0x00027c0202020200, 0x00047a0404040400, 0x0008760808080800, 
        0x00106e1010101000, 0x00205e2020202000, 0x00403e4040404000, 0x00807e8080808000, 
        0x007e010101010100, 0x007c020202020200, 0x007a040404040400, 0x0076080808080800, 
        0x006e101010101000, 0x005e202020202000, 0x003e404040404000, 0x007e808080808000, 
        0x7e01010101010100, 0x7c02020202020200, 0x7a04040404040400, 0x7608080808080800, 
        0x6e10101010101000, 0x5e20202020202000, 0x3e40404040404000, 0x7e80808080808000
    };

    /// @brief Bishop movement masks, used for sliding piece attacks
    alignas(64) inline constexpr array<BB, 64> bishop_movement_masks = {
        0x0040201008040200, 0x0000402010080400, 0x0000004020100a00, 0x0000000040221400, 
        0x0000000002442800, 0x0000000204085000, 0x0000020408102000, 0x0002040810204000, 
        0x0020100804020000, 0x0040201008040000, 0x00004020100a0000, 0x0000004022140000, 
        0x0000000244280000, 0x0000020408500000, 0x0002040810200000, 0x0004081020400000, 
        0x0010080402000200, 0x0020100804000400, 0x004020100a000a00, 0x0000402214001400, 
        0x0000024428002800, 0x0002040850005000, 0x0004081020002000, 0x0008102040004000, 
        0x0008040200020400, 0x0010080400040800, 0x0020100a000a1000, 0x0040221400142200, 
        0x0002442800284400, 0x0004085000500800, 0x0008102000201000, 0x0010204000402000, 
        0x0004020002040800, 0x0008040004081000, 0x00100a000a102000, 0x0022140014224000, 
        0x0044280028440200, 0x0008500050080400, 0x0010200020100800, 0x0020400040201000, 
        0x0002000204081000, 0x0004000408102000, 0x000a000a10204000, 0x0014001422400000, 
        0x0028002844020000, 0x0050005008040200, 0x0020002010080400, 0x0040004020100800, 
        0x0000020408102000, 0x0000040810204000, 0x00000a1020400000, 0x0000142240000000, 
        0x0000284402000000, 0x0000500804020000, 0x0000201008040200, 0x0000402010080400, 
        0x0002040810204000, 0x0004081020400000, 0x000a102040000000, 0x0014224000000000, 
        0x0028440200000000, 0x0050080402000000, 0x0020100804020000, 0x0040201008040200
    };

    /// @brief Magic tables for rook movements
    alignas(64) inline array<array<BB, 4096>, 64> rook_movement_table;

    /// @brief Magic tables for bishop movements
    alignas(64) inline array<array<BB, 512>, 64> bishop_movement_table;

    /// @brief Generate a variation mask for sliding piece attacks
    inline BB generate_variation_mask(int idx, int pop_c, BB mask) {
        BB bb = 0ULL;
        for (int variation = 0; variation < pop_c; variation++) {
            int sq = bitboard_utils::bitscan_forward(mask);
            bitboard_utils::pop_bit(mask, sq);
            if (idx & (1 << variation)) 
                bb |= (1ULL << sq);
        }

        return bb;
    }

    /// @brief Generate blocked attacks for rook pieces
    inline BB rook_blocked_attacks(int sq, BB occ) {
        BB slider = bitboard_utils::mask(sq);
        BB attacks = 0;

        attacks |= sliding_attacks(slider, occ, bitboard_utils::nort);
        attacks |= sliding_attacks(slider, occ, bitboard_utils::sout);
        attacks |= sliding_attacks(slider, occ, bitboard_utils::east);
        attacks |= sliding_attacks(slider, occ, bitboard_utils::west);

        return attacks;
    }

    /// @brief Generate blocked attacks for bishop pieces
    inline BB bishop_blocked_attacks(int sq, BB occ) {
        BB slider = bitboard_utils::mask(sq);
        BB attacks = 0;

        attacks |= sliding_attacks(slider, occ, bitboard_utils::noEast);
        attacks |= sliding_attacks(slider, occ, bitboard_utils::noWest);
        attacks |= sliding_attacks(slider, occ, bitboard_utils::soEast);
        attacks |= sliding_attacks(slider, occ, bitboard_utils::soWest);

        return attacks;
    }

    /// @brief Initialize sliding move tables for rooks and bishops
    inline void init_sliding_move_tables() {
        for (int sq = 0; sq < 64; sq++) {
            BB attack_mask = rook_movement_masks[sq];
            int bits = bitboard_utils::pop_count(attack_mask);
            int variation_count = 1 << bits;
            for (int i = 0; i < variation_count; i++) {
                BB variation = generate_variation_mask(i, bits, attack_mask);
                int magic_idx = (variation * rook_magics[sq]) >> (64 - rook_shifts[sq]);
                rook_movement_table[sq][magic_idx] = rook_blocked_attacks(sq, variation);
            }

            attack_mask = bishop_movement_masks[sq];
            bits = bitboard_utils::pop_count(attack_mask);
            variation_count = 1 << bits;
            for (int i = 0; i < variation_count; i++) {
                BB variation = generate_variation_mask(i, bits, attack_mask);
                int magic_idx = (variation * bishop_magics[sq]) >> (64 - bishop_shifts[sq]);
                bishop_movement_table[sq][magic_idx] = bishop_blocked_attacks(sq, variation);
            }
        }
    }

    inline BB rook_moves(int sq, BB bb) {

        // Gets rook moves for a given square and blocker bitboard.
        
        bb &= rook_movement_masks[sq];
        bb *= rook_magics[sq];
        bb >>= 64 - rook_shifts[sq];
        return rook_movement_table[sq][bb];
    }

    inline BB bishop_moves(int sq, BB bb) {

        // Gets bishop moves for a given square and blocker bitboard.

        bb &= bishop_movement_masks[sq];
        bb *= bishop_magics[sq];
        bb >>= 64 - bishop_shifts[sq];
        return bishop_movement_table[sq][bb];
    }

    inline BB queen_moves(int sq, BB bb) {

        // Gets queen moves for a given square and blocker bitboard.

        return rook_moves(sq, bb) | bishop_moves(sq, bb);
    }
} 

#endif