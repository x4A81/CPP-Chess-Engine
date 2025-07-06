#ifndef UCI_HPP_INCLUDE
#define UCI_HPP_INCLUDE

#include <string>
#include <vector>
#include <cstdint>

using namespace std;

using Move = uint16_t;

bool handle_command(const string& command);
void send_info();
vector<string> get_tokens(const string& command);
Move parse_move_string(const string move_str);
void handle_go(const string& command);
void setup_engine();
void clean();

#endif