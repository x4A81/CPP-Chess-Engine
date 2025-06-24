#include "../include/board.h"
#include "../include/tests.h"

int main() {
    Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ");
    board.print_board();
    tests::test_board = board;
    tests::test(1, 6);

    return 0;
}