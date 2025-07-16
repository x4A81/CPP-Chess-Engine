#include <vector>
#include <sstream>
#include <thread>
#include <print>
#include <string>

#include "../include/uci.hpp"
#include "../include/utils.hpp"
#include "../include/board.hpp"
#include "../include/search.hpp"
#include "../include/transposition.hpp"
#include "../include/tests.hpp"
#include "../include/book.hpp"

bool is_board_initialised = false;

std::vector<std::string> get_tokens(const std::string& command) {
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string this_token;
    while (iss >> this_token)
        tokens.push_back(this_token);
    
    return tokens;
}

void stop_search() {
    stop_flag = true;
}

Move parse_move_string(const std::string move_str) {
    Move move = nullmove;
    Code code = 0;

    BoardState state = game_board.state;

    int file = move_str[0] - 'a';
    int rank = move_str[1] - '1';
    Square from_sq = rank * 8 + file;

    file = move_str[2] - 'a';
    rank = move_str[3] - '1';
    Square to_sq = rank * 8 + file;

    bool is_pawn = (state.piece_list[from_sq] == P || state.piece_list[from_sq] == p);
    
    if (is_pawn && (to_sq - from_sq == 16 || to_sq - from_sq == -16))
        code = dbpush;

    if (state.piece_list[from_sq] == K && from_sq == e1) {
        if (to_sq == g1) code = kcastle;
        if (to_sq == c1) code = qcastle;

        // Some UCI engines use e1h1 for short castling.
        // This is wrong but supported here anyway.
        if (to_sq == h1) {
            code = kcastle;
            to_sq = g1;
        }

        if (to_sq == a1) {
            code = qcastle;
            to_sq = c1;
        }
    }

    if (state.piece_list[from_sq] == k && from_sq == e8) {
        if (to_sq == g8) code = kcastle;
        if (to_sq == c8) code = qcastle;

        if (to_sq == h8) {
            code = kcastle;
            to_sq = g8;
        }

        if (to_sq == a8) {
            code = qcastle;
            to_sq = c8;
        }
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

    move = from_sq | (to_sq << 6);
    return move | (code << 12);
}

void send_info() {
    std::print("id name Chess Engine\nid author x4A81\n");
    std::print("option name Hash type spin default {} min {} max {}\n", 
        (MAX_TT_SIZE_MB+MIN_TT_SIZE_MB)/2, MIN_TT_SIZE_MB, MAX_TT_SIZE_MB);
    std::println("uciok");
}

void setup_engine() {
    if (!is_board_initialised) game_board = Board(1);
    // setup transposition table and search thread
    if (!game_table.has_value()) game_table.emplace((MAX_TT_SIZE_MB+MIN_TT_SIZE_MB)/2);
}

void clean() {
    stop_flag.store(true);
}

void handle_go(const std::string& command) {
    std::vector<std::string> tokens = get_tokens(command);
    bool go_perft = false;
    int perft_depth = 0;

    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& tok = tokens[i];
        if (tok == "depth" && i + 1 < tokens.size())
            game_board.search_params.max_depth = stoi(tokens[++i]);
        else if (tok == "movetime" && i + 1 < tokens.size())
            game_board.search_params.move_time = stoi(tokens[++i]);
        else if (tok == "movestogo" && i + 1 < tokens.size()) {
            game_board.search_params.movestogo = stoi(tokens[++i]);
            game_board.search_params.max_depth = UNUSED;
            game_board.search_params.move_time = UNUSED;
            game_board.search_params.nodes = UNUSED;
            game_board.search_params.infinite = false;
        }
        else if (tok == "nodes" && i + 1 < tokens.size())
            game_board.search_params.nodes = stoi(tokens[++i]);
        else if (tok == "infinite")
            game_board.search_params.infinite = true;
        else if (tok == "wtime" && i + 1 < tokens.size()) {
            game_board.search_params.wtime = stoi(tokens[++i]);
            game_board.search_params.max_depth = UNUSED;
            game_board.search_params.move_time = UNUSED;
            game_board.search_params.nodes = UNUSED;
            game_board.search_params.infinite = false;
        }
        else if (tok == "btime" && i + 1 < tokens.size()) {
            game_board.search_params.btime = stoi(tokens[++i]);
            game_board.search_params.max_depth = UNUSED;
            game_board.search_params.move_time = UNUSED;
            game_board.search_params.nodes = UNUSED;
            game_board.search_params.infinite = false;
        }
        else if (tok == "winc" && i + 1 < tokens.size()) {
            game_board.search_params.winc = stoi(tokens[++i]);
            game_board.search_params.max_depth = UNUSED;
            game_board.search_params.move_time = UNUSED;
            game_board.search_params.nodes = UNUSED;
            game_board.search_params.infinite = false;
        }
        else if (tok == "binc" && i + 1 < tokens.size()) {
            game_board.search_params.binc = stoi(tokens[++i]);
            game_board.search_params.max_depth = UNUSED;
            game_board.search_params.move_time = UNUSED;
            game_board.search_params.nodes = UNUSED;
            game_board.search_params.infinite = false;
        }
        else if (tok == "perft" && i + 1 < tokens.size()) {
            perft_depth = stoi(tokens[++i]);
            go_perft = true;
        }
    }

    std::println("info string depth {} movetime {} movestogo {} infinite {} wtime {} winc {} btime {} binc {}",
    game_board.search_params.max_depth,
    game_board.search_params.move_time,
    game_board.search_params.movestogo,
    game_board.search_params.infinite,
    game_board.search_params.wtime,
    game_board.search_params.winc,
    game_board.search_params.btime,
    game_board.search_params.binc
    );

 
    if (go_perft) {
        tests::test_board = game_board;
        tests::test(perft_depth);
        return;
    }

    stop_flag.store(false);
    std::thread search_thread([]() { game_board.run_search(); });
    search_thread.detach();
}

bool handle_command(const std::string& command) {
    if (command == "quit") {
        clean();
        return true;
    }

    if (command == "uci")
        send_info();

    if (command == "stop")
        stop_flag.store(true);

    if (command == "isready") {
        setup_engine();
        std::println("readyok");
    }

    if (command.starts_with("go"))
        handle_go(command);

    if (command == "ucinewgame") {
        game_board = Board(1);
        game_board.clean_search();
    }

    if (command.starts_with("setoption")) {
        std::vector<std::string> tokens = get_tokens(command);
        std::string name, value;
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
            std::println("info string Hash set to {} MB", mb);
        }
    }

    if (command.starts_with("position")) {
        std::vector<std::string> tokens = get_tokens(command);
        int token_idx = 1;
        bool parsing_moves = false;
        while (token_idx < tokens.size()) {

            std::string this_token = tokens.at(token_idx);
            if (parsing_moves) {
                game_board.make_move(parse_move_string(this_token));
            } else {
                if (this_token == "startpos") {
                    game_board = Board(1);
                    is_board_initialised = true;
                }
                
                else if (this_token == "fen") {
                    // FEN is always 6 tokens: piece placement, active color, castling, en passant, halfmove, fullmove
                    std::string fen;
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

    if (command == "eval") {
        std::println("info score cp {}",
            (game_board.state.side_to_move == white) ? game_board.eval() : -game_board.eval()
        ); // Negate the eval as viewed from white
    }

    if (command == "usage") {
        std::println("info string tt_usage {}%", game_table->usage());
    }

    if (command == "bookmoves") {
        std::vector<polyglot::BookEntry> entries = polyglot::probe_book(polyglot::gen_poly_key(game_board.state));

        if (!entries.empty()) {
            for (auto& pos : entries) {
                std::print("info string {}", move_to_string(polyglot::get_book_move(pos, game_board.state)));
                std::println(" weight: {} learn: {}", pos.weight, pos.learn);
            }
        } else {
            std::println("info string no book entries");
        }
    }

    return false;
}