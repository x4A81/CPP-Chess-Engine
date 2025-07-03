#include "../include/uci.hpp"
#include "../include/utils.hpp"
#include "../include/board.hpp"
#include "../include/search.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>

using namespace std;
using namespace uci;

Board global_board;
bool is_board_initialised = false;

vector<string> uci::get_tokens(const string& command) {
    istringstream iss(command);
    vector<string> tokens;
    string this_token;
    while (iss >> this_token)
        tokens.push_back(this_token);
    
    return tokens;

}

Move uci::parse_move_string(const string move_str) {
    Move move = nullmove;
    int code = 0;

    Board_State state = global_board.state;

    int file = move_str[0] - 'a';
    int rank = move_str[1] - '1';
    int from_sq = rank * 8 + file;

    cout << square_to_string(Squares(from_sq)) << endl;
    

    file = move_str[2] - 'a';
    rank = move_str[3] - '1';
    int to_sq = rank * 8 + file;
    move = from_sq | (to_sq << 6);

    bool is_pawn = (state.piece_list[from_sq] == P || state.piece_list[from_sq] == p);
    
    if (is_pawn && (to_sq - from_sq == 16 || to_sq - from_sq == -16))
        code = dbpush;

    if (state.piece_list[from_sq] == K && from_sq == e1) {
        if (to_sq == g1) code = kcastle;
        if (to_sq == c1) code = qcastle;
    }

    if (state.piece_list[from_sq] == k && from_sq == e8) {
        if (to_sq == g8) code = kcastle;
        if (to_sq == c8) code = qcastle;
    }

    if (state.piece_list[to_sq] != no_piece)
        code = capture;

    if ((to_sq == state.enpassant_square) && is_pawn)
        code = epcapture;

    if (move_str.length() == 5 && is_pawn &&
        (rank == 0 || rank == 7)) // promotion rank
    {
        char promo = move_str[4];
        switch (promo) {
            case 'q': code |= qpromo; break;
            case 'r': code |= rpromo; break;
            case 'b': code |= bpromo; break;
            case 'n': code |= npromo; break;
        }
    }

    return move | (code << 12);
}

void uci::send_info() {
    cout << "id name Chess Engine" << endl;
    cout << "id author x4A81" << endl;
}

void uci::setup_engine() {
    if (!is_board_initialised) global_board = Board(1);
    // setup transposition table and search thread
}

void uci::clean() {
    stop_search();
    // clear transpositoin table and search thread
}

void uci::stop_search() {
    stop_flag = true;
}

void uci::handle_go(const string& command) {
    SearchParams params;
    params.max_depth = 8;
    thread search_thread([params]() { run_search(params); });
    search_thread.detach();
}

bool uci::handle_command(const string& command) {
    if (command == "quit")
        return false;

    if (command == "uci") {
        send_info();
        cout << "uciok" << endl;
    }

    if (command == "isready") {
        setup_engine();
        cout << "readyok" << endl;
    }

    if (command.starts_with("go"))
        handle_go(command);

    if (command.starts_with("position")) {
        vector<string> tokens = get_tokens(command);
        int token_idx = 1;
        bool parsing_moves = false;
        while (token_idx < tokens.size()) {

            string this_token = tokens.at(token_idx);
            if (parsing_moves) {
                global_board.print_board();
                global_board.make_move(parse_move_string(this_token));
            } else {
                if (this_token == "startpos") {
                    global_board = Board(1);
                    is_board_initialised = true;
                }
                
                else if (this_token == "fen") {
                    // FEN is always 6 tokens: piece placement, active color, castling, en passant, halfmove, fullmove
                    string fen;
                    for (int i = 1; i <= 6 && (token_idx + i) < tokens.size(); ++i) {
                        fen += tokens.at(token_idx + i);
                        if (i < 6) fen += " ";
                    }
                    global_board = Board(fen);
                    token_idx += 6;

                    is_board_initialised = true;
                }

                else if (this_token == "moves")
                    parsing_moves = true;
            }

            token_idx++;
        }

    }

    if (command == "d") global_board.print_board();

    return true;
}