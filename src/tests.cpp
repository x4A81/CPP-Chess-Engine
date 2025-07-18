#include "../include/tests.hpp"
#include "../include/board.hpp"
#include "../include/utils.hpp"

#include <print>
#include <chrono>

namespace tests {

    long positions_searched = 0;
    Board test_board;

    /// Runs a normal perft test on test_board. See https://www.chessprogramming.org/Perft#Perft_function.
    void perft(int depth) {
        if (depth <= 0) {
            positions_searched++;
            return;
        }

        test_board.generate_moves<ALLMOVES>();

        for (Move move : test_board.state.move_list) {
            test_board.make_move(move);
            perft(depth - 1);
            test_board.unmake_last_move();
        }
    }

    /// Runs a perft test with optional divide. See https://www.chessprogramming.org/Perft#Divide.
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
                std::print("{}: {}\n", move_to_string(move), positions_searched - c_positions);
            }
            test_board.unmake_last_move();
        }
    }

    /// Runs a perft divide test at one depth.
    void test(int depth) {
        positions_searched = 0;
        std::println("Running at depth {}", depth);

        auto start = std::chrono::high_resolution_clock::now();
        perft_test(depth, true);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        std::println("Positions searched: {}\nTime taken: {}s\nNPS: {}/s",
                     positions_searched, elapsed.count(), positions_searched / elapsed.count());
    }

    /// Runs a perft test across a range of depths.
    void test(int start, int stop, bool divide) {
        for (int depth = start; depth <= stop; ++depth) {
            std::println("Depth: {}", depth);
            positions_searched = 0;
            perft_test(depth, divide);
            std::println("Positions searched: {}", positions_searched);
        }
    }

    /// Runs multiple perft tests on known positions.
    void perft_suite() {
        std::println("Initial Position");
        test_board = Board(1);
        test(1, 4, false);

        std::println("\nKiwipete Position");
        test_board.load_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0");
        test(1, 4, false);

        std::println("\nEn Passant Discovered Check Position");
        test_board.load_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        test(1, 4, false);

        std::println("\nProblematic Position");
        test_board.load_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        test(1, 4, false);
    }

}
