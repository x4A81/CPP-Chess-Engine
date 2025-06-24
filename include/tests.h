#ifndef TESTS_H_INCLUDE
#define TESTS_H_INCLUDE

#include <iostream>
#include "../include/board.h"
#include "../include/utils.h"

namespace tests {
    long positions_searched = 0;
    Board test_board;
    void perft(int depth) {
        if (depth <= 0) {
            positions_searched++;
            return;
        }

        test_board.generate_moves<ALLMOVES>();

        for (Move move : test_board.state.move_list) {
            test_board.make_move(move);
            perft(depth-1);
            test_board.unmake_last_move();
        }
    }

    void perft_test(int depth) {
        if (depth <= 0) {
            positions_searched++;
            return;
        }

        test_board.generate_moves<ALLMOVES>();

        for (Move move : test_board.state.move_list) {
            test_board.make_move(move);
            long c_positions = positions_searched;
            print_move(move);
            perft(depth - 1);
            std::cout << ": " << positions_searched - c_positions << std::endl;
            test_board.unmake_last_move();
        }
    }

    void test(int depth) {
        positions_searched = 0;
        perft_test(depth);
        std::cout << "Positions searched: " << positions_searched << std::endl;
    }

    void test(int start, int stop) {
        for (int depth = start; depth <= stop; depth++) {
            std::cout << "Depth: " << depth << std::endl;
            positions_searched = 0;
            perft_test(depth);
            std::cout << "Positions searched: " << positions_searched << std::endl;
        }
    }
}


#endif