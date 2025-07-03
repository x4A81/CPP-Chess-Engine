#ifndef TESTS_H_INCLUDE
#define TESTS_H_INCLUDE

#include "../include/board.hpp"
#include "../include/utils.hpp"
#include <iostream>

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

    void perft_test(int depth, int divide) {
        if (depth <= 0) {
            positions_searched++;
            return;
        }

        test_board.generate_moves<ALLMOVES>();

        for (Move move : test_board.state.move_list) {
            test_board.make_move(move);
            long c_positions = positions_searched;
            perft(depth - 1);
            if (divide) {
                print_move(move);
                std::cout << ": " << positions_searched - c_positions << std::endl;
            }

            test_board.unmake_last_move();
        }
    }

    void test(int depth) {
        positions_searched = 0;
        perft_test(depth, 1);
        std::cout << "Positions searched: " << positions_searched << std::endl;
    }

    void test(int start, int stop, int divide = 1) {
        for (int depth = start; depth <= stop; depth++) {
            std::cout << "Depth: " << depth << std::endl;
            positions_searched = 0;
            perft_test(depth, divide);
            std::cout << "Positions searched: " << positions_searched << std::endl;
        }
    }

    void perft_suite() {
        std::cout << "Initial Position" << std::endl;
        test_board = Board(1);
        test(1, 4, 0);

        std::cout << "\nKiwipete Position" << std::endl;
        test_board.load_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0");
        test(1, 4, 0);
        
        std::cout << "\nEnpassant Discovered check Position" << std::endl;
        test_board.load_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        test(1, 4, 0);
        
        std::cout << "\nProblematic Position" << std::endl;
        test_board.load_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        test(1, 4, 0);
    }
}


#endif