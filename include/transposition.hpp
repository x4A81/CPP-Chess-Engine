#ifndef TRANSPOSITION_HPP_INCLUDE
#define TRANSPOSITION_HPP_INCLUDE

#include "board.hpp"
#include <cstddef>
#include <optional>

enum EntryType {
    EXACT,
    LOWER,
    UPPER
};

struct alignas(64) TranspositionEntry {
    KEY key;
    Move hash_move;
    int depth;
    int score;
    EntryType type;
};

constexpr std::size_t MIN_TT_SIZE_MB = 64;
constexpr std::size_t MAX_TT_SIZE_MB = 512;
constexpr std::size_t TT_ENTRY_SIZE = sizeof(TranspositionEntry);
constexpr std::size_t MIN_TT_SIZE = (MIN_TT_SIZE_MB * 1024 * 1024) / TT_ENTRY_SIZE;
constexpr std::size_t MAX_TT_SIZE = (MAX_TT_SIZE_MB * 1024 * 1024) / TT_ENTRY_SIZE;

class Transposition {
private:
    TranspositionEntry* transposition_tt = nullptr;
    std::size_t transposition_size = 0;
    void init(std::size_t size);
public:
    Transposition(std::size_t size = 0) {
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
    TranspositionEntry* probe(KEY key, int depth);
    void store_entry(TranspositionEntry& entry);
};

extern std::optional<Transposition> game_table;

#endif