#ifndef BITBOARD_UTILS_H_INCLUDE
#define BITBOARD_UTILS_H_INCLUDE

#include "board.h"

#include <array>
#include <bit>

using namespace std;

namespace bitboard_utils {
    const BB AFILE = BB(0x0101010101010101);
    const BB HFILE = BB(0x8080808080808080);
    const BB ABFILE = BB(0x0303030303030303);
    const BB GHFILE = BB(0xC0C0C0C0C0C0C0C0);

    const BB nAFILE = ~AFILE;
    const BB nHFILE = BB(0x7F7F7F7F7F7F7F7F);
    const BB nABFILE = ~ABFILE;
    const BB nGHFILE = ~GHFILE;

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

    const std::array<BB, 8> avoid_wraps = { 
        BB(0xFFFFFFFFFFFFFF00), BB(0x00FFFFFFFFFFFFFF), BB(0xFEFEFEFEFEFEFEFE), BB(0x7F7F7F7F7F7F7F7F), 
        BB(0xFEFEFEFEFEFEFE00), BB(0x7F7F7F7F7F7F7F00), BB(0x00FEFEFEFEFEFEFE), BB(0x007F7F7F7F7F7F7F) 
    };

    const std::array<int, 16> shifts = { 8,  -8,  1,  -1,  9, 7,  -7,  -9,
                                         17, 10, -6, -15, 15, 6, -10, -17 };

    inline BB mask(int sq) { return (BB(1) << sq); }
    inline void pop_bit(BB& bb, int sq) { bb &= ~mask(sq); }
    inline void set_bit(BB& bb, int sq) { bb |= mask(sq); }
    inline int get_bit(BB bb, int sq) { return (bb >> sq) & 1; }
    inline int bitscan_forward(BB bb) { return countr_zero(bb); }
    
    inline BB shift_one(BB bb, Dir dir) {
        int s = shifts[dir];
        return rotl(bb, s) & avoid_wraps[dir];
    }

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

    inline BB sliding_attacks(BB sliders, BB occ, Dir dir) {
        return shift_one(occ_fill(sliders, ~occ, dir), dir);
    }
}

#endif