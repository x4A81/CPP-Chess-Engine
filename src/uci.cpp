#include "../include/uci.hpp"
#include "../include/utils.hpp"
#include "../include/board.hpp"
#include "../include/search.hpp"
#include "../include/transposition.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>

using namespace std;

bool is_board_initialised = false;

vector<string> get_tokens(const string& command) {
    istringstream iss(command);
    vector<string> tokens;
    string this_token;
    while (iss >> this_token)
        tokens.push_back(this_token);
    
    return tokens;
}

void stop_search() {
    stop_flag = true;
}

Move parse_move_string(const string move_str) {
    Move move = nullmove;
    int code = 0;

    Board_State state = game_board.state;

    int file = move_str[0] - 'a';
    int rank = move_str[1] - '1';
    int from_sq = rank * 8 + file;

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

void send_info() {
    cout << "id name Chess Engine" << endl;
    cout << "id author x4A81" << endl;
    cout << "option name Hash type spin default " << (MAX_TT_SIZE_MB+MIN_TT_SIZE_MB)/2 << " min " << MIN_TT_SIZE_MB << " max " << MAX_TT_SIZE_MB << endl;
    cout << "uciok" << endl;
}

void setup_engine() {
    if (!is_board_initialised) game_board = Board(1);
    // setup transposition table and search thread
    if (!game_table.has_value()) game_table.emplace((MAX_TT_SIZE_MB+MIN_TT_SIZE_MB)/2);
}

void clean() {
    stop_flag = true;
    // clear transpositoin table and search thread
    // game_table.clear_tt();
}

void handle_go(const string& command) {
    SearchParams params;
    vector<string> tokens = get_tokens(command);

    if (tokens.size() == 1) return;

    for (size_t i = 1; i < tokens.size(); ++i) {
        const string& tok = tokens[i];
        if (tok == "depth" && i + 1 < tokens.size())
            params.max_depth = stoi(tokens[++i]);
        else if (tok == "movetime" && i + 1 < tokens.size())
            params.move_time = stoi(tokens[++i]);
        else if (tok == "nodes" && i + 1 < tokens.size())
            params.nodes = stoi(tokens[++i]);
        else if (tok == "infinite")
            params.infinite = true;
        else if (tok == "wtime" && i + 1 < tokens.size())
            params.total_time = stoi(tokens[++i]);
        else if (tok == "btime" && i + 1 < tokens.size())
            params.total_time = stoi(tokens[++i]);
        else if (tok == "winc" && i + 1 < tokens.size())
            params.inc = stoi(tokens[++i]);
        else if (tok == "binc" && i + 1 < tokens.size())
            params.inc = stoi(tokens[++i]);
    }

    stop_flag = false;
    thread search_thread([params]() { game_board.run_search(params); });
    search_thread.detach();
}

bool handle_command(const string& command) {
    if (command == "quit") {
        clean();
        return false;
    }

    if (command == "uci")
        send_info();

    if (command == "stop")
        stop_flag = true;

    if (command == "isready") {
        setup_engine();
        cout << "readyok" << endl;
    }

    if (command.starts_with("go"))
        handle_go(command);

    if (command.starts_with("setoption")) {
        vector<string> tokens = get_tokens(command);
        string name, value;
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i] == "name" && i + 1 < tokens.size()) {
                name = tokens[++i];
            } else if (tokens[i] == "value" && i + 1 < tokens.size()) {
                value = tokens[++i];
            }
        }

        if (name == "Hash" && !value.empty()) {
            int mb = stoi(value);
            mb = std::clamp(mb, int(MIN_TT_SIZE * TT_ENTRY_SIZE / (1024 * 1024)), int(MAX_TT_SIZE * TT_ENTRY_SIZE / (1024 * 1024)));
            std::size_t entries = (mb * 1024 * 1024) / TT_ENTRY_SIZE;
            game_table.emplace(entries);
            cout << "info string Hash set to " << mb << " MB" << endl;
        }
    }

    if (command.starts_with("position")) {
        vector<string> tokens = get_tokens(command);
        int token_idx = 1;
        bool parsing_moves = false;
        while (token_idx < tokens.size()) {

            string this_token = tokens.at(token_idx);
            if (parsing_moves) {
                game_board.make_move(parse_move_string(this_token));
            } else {
                if (this_token == "startpos") {
                    game_board = Board(1);
                    is_board_initialised = true;
                }
                
                else if (this_token == "fen") {
                    // FEN is always 6 tokens: piece placement, active color, castling, en passant, halfmove, fullmove
                    string fen;
                    for (int i = 1; i <= 6 && (token_idx + i) < tokens.size(); ++i) {
                        fen += tokens.at(token_idx + i);
                        if (i < 6) fen += " ";
                    }

                    game_board = Board(fen);
                    token_idx += 6;

                    is_board_initialised = true;
                }

                else if (this_token == "moves")
                    parsing_moves = true;
            }

            token_idx++;
        }

    }

    if (command == "d") game_board.print_board();

    return true;
}