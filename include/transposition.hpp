#ifndef TRANSPOSITION_HPP_INCLUDE
#define TRANSPOSITION_HPP_INCLUDE

#include <cstddef>
#include <optional>

#include "board.hpp"

class Transposition;
extern std::optional<Transposition> game_table;

enum EntryType {
    EXACT,
    LOWER,
    UPPER
};

struct alignas(64) TranspositionEntry {
    Key key;
    Move hash_move;
    int depth;
    int age;
    Score score;
    EntryType type;
};

constexpr size_t MIN_TT_SIZE_MB = 64;
constexpr size_t MAX_TT_SIZE_MB = 512;
constexpr size_t TT_ENTRY_SIZE = sizeof(TranspositionEntry);
constexpr size_t MIN_TT_SIZE = (MIN_TT_SIZE_MB * 1024 * 1024) / TT_ENTRY_SIZE;
constexpr size_t MAX_TT_SIZE = (MAX_TT_SIZE_MB * 1024 * 1024) / TT_ENTRY_SIZE;

class Transposition {
private:
    TranspositionEntry* transposition_tt = nullptr;
    size_t transposition_size = 0;
    void init(size_t size);
public:
    Transposition(size_t size = 0) {
        init(size);
    }

    ~Transposition() {
        if (transposition_tt) {
            delete[] transposition_tt;
            transposition_size = 0;
        }
    }

    Transposition(const Transposition&) = delete;
    Transposition& operator=(const Transposition&) = delete;

    bool is_initialised = false;

    void clear_tt();
    TranspositionEntry* probe(Key key, int depth);
    void store_entry(TranspositionEntry& entry);
    float usage() const;
};

#endif