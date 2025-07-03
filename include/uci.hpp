#ifndef UCI_HPP
#define UCI_HPP

#include <string>
#include <vector>
#include <cstdint>

using namespace std;

using Move = uint16_t;

namespace uci {
    bool handle_command(const string& command);
    void send_info();
    vector<string> get_tokens(const string& command);
    Move parse_move_string(const string move_str);
    void handle_go(const string& command);
    void stop_search();
    void setup_engine();
    void clean();
}

#endif