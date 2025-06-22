#include "../include/board.h"
#include "../tests/tests.h"

int main() {
    Board board("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 0 1 ");
    board.print_board();
    tests::test_board = board;
    tests::test(1);

    return 0;
}