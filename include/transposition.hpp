/*


UNUSED

TEMPLATE OF FUTURE IMPLEMENTATION OF TRANPOSITION TABLE


*/

#ifndef TRANSPOSITION_H_INCLUDE
#define TRANSPOSITION_H_INCLUDE

#include "board.hpp"
#include <cstddef>

enum EntryType {
    EXACT,
    LOWER,
    UPPER
};

struct alignas(64) TRANSPOSITION_ENTRY {
    KEY key;
    Move hash_move;
    int depth;
    int score;
    EntryType type;
};

class Transposition {
private:
    TRANSPOSITION_ENTRY* transposition_tt;
    std::size_t transposition_size;
    void init(std::size_t size);
public:
    Transposition(std::size_t size = 0) : transposition_tt(nullptr), transposition_size(size) {
        if (size > 0) {
            init(size);
        }
    }

    ~Transposition() {
        if (transposition_tt) {
            delete[] transposition_tt;
            transposition_size = 0;
        }
    }

    void clear();
    TRANSPOSITION_ENTRY* probe(KEY key);
    void store_entry(const TRANSPOSITION_ENTRY& entry);
};

#endif