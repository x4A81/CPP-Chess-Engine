#include <algorithm>
#include <cassert>

#include "../include/transposition.hpp"

void Transposition::init(std::size_t size) {
    if (transposition_tt) {
        delete[] transposition_tt;
        transposition_tt = nullptr;
        transposition_size = 0;
    }

    if (size == 0) return;

    // Convert min and max TT entries:
    constexpr size_t MIN_TT_ENTRIES = MIN_TT_SIZE;
    constexpr size_t MAX_TT_ENTRIES = MAX_TT_SIZE;

    size = std::clamp(size, MIN_TT_ENTRIES, MAX_TT_ENTRIES);
    std::size_t power = 1;
    while (power < size && (power << 1) > 0) power <<= 1; // Round up to nearest power of 2
    size = power;
    while (transposition_size == 0 && size >= MIN_TT_SIZE_MB) {
        try {
            transposition_tt = new TranspositionEntry[size]();
            transposition_size = size;
        } catch (const std::bad_alloc&) {
            transposition_tt = nullptr;
            transposition_size = 0;
            size /= 2;
        }
    }

    is_initialised = true;
}

void Transposition::clear_tt() {
    if (transposition_tt && transposition_size > 0)
        std::fill_n(transposition_tt, transposition_size, TranspositionEntry{});
}

TranspositionEntry* Transposition::probe(Key key, int depth) {
    if (!transposition_tt || transposition_size == 0)
        return nullptr;
    
    int index = key & (transposition_size - 1);
    TranspositionEntry *entry = &transposition_tt[index];

    assert(index < transposition_size);

    if (entry->key == key && entry->depth >= depth)
        return entry;
    return nullptr;
}

void Transposition::store_entry(TranspositionEntry& entry) {
    if (!transposition_tt || transposition_size == 0)
        return;

    int index = entry.key & (transposition_size - 1);
    TranspositionEntry *table_entry = &transposition_tt[index];

    // Depth preferred replacement strategy.
    if ((table_entry->key == entry.key && table_entry->depth < entry.depth) || table_entry->key == 0)
        *table_entry = entry;
}

float Transposition::usage() const {
    if (!is_initialised || transposition_size == 0) return 0.0f;

    size_t occupied = 0;
    for (size_t i = 0; i < transposition_size; ++i) {
        if (transposition_tt[i].key != 0) {
            occupied++;
        }
    }

    return 100.0f * occupied / transposition_size;
}