#ifndef TESTS_HPP_INCLUDE
#define TESTS_HPP_INCLUDE

#include <print>

#include "../include/board.hpp"
#include "../include/utils.hpp"

namespace tests {
    long positions_searched = 0;
    Board test_board;

    /// @brief Runs a normal perft test on test_board. See https://www.chessprogramming.org/Perft#Perft_function.
    /// @param depth Depth to test.
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

    /// @brief Runs a perft test with optional divide. See https://www.chessprogramming.org/Perft#Divide.
    /// @param depth Depth to test.
    /// @param divide If true, then run a perft divide test.
    void perft_test(int depth, bool divide) {
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
                std::print(": {}\n", positions_searched - c_positions);
            }

            test_board.unmake_last_move();
        }
    }

    /// @brief Runs a perft divide test.
    /// @param depth Depth to test.
    void test(int depth) {
        positions_searched = 0;
        perft_test(depth, true);
        std::println("Positions searched: ", positions_searched);
    }

    /// @brief Runs a perft test, with optional divide.
    /// @param start Depth to start test.
    /// @param stop Depth to stop test.
    /// @param divide If true, then run perft divide, default true.
    void test(int start, int stop, bool divide = true) {
        for (int depth = start; depth <= stop; ++depth) {
            std::println("Depth: ", depth);
            positions_searched = 0;
            perft_test(depth, divide);
            std::println("Positions searched: ", positions_searched);
        }
    }

    /// @brief Runs multiple perft tests on predetermined positions.
    void perft_suite() {
        std::println("Initial Position");
        test_board = Board(1);
        test(1, 4, 0);

        std::println("\nKiwipete Position");
        test_board.load_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0");
        test(1, 4, 0);
        
        std::print("\nEnpassant Discovered check Position");
        test_board.load_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        test(1, 4, 0);
        
        std::println("\nProblematic Position");
        test_board.load_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        test(1, 4, 0);
    }
}

#endif