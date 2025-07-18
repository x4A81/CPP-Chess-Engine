#ifndef TESTS_HPP_INCLUDE
#define TESTS_HPP_INCLUDE

#include <cstdint>

class Board;

namespace tests {
    extern long positions_searched;
    extern Board test_board;
    void perft(int depth);
    void perft_test(int depth, bool divide);
    void test(int depth);
    void test(int start, int stop, bool divide = true);
    void perft_suite();

}

#endif