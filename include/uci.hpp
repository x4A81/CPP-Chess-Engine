#ifndef UCI_HPP_INCLUDE
#define UCI_HPP_INCLUDE

#include <string>
#include <vector>
#include <cstdint>

using namespace std;
using Move = uint16_t;

/// @brief UCI command handler.
/// @param command UCI command.
/// @return true if received 'quit' command.
bool handle_command(const string& command);

/// @brief Prints engine info to std::cout.
void send_info();

/// @brief Tokenises a UCI command.
/// @param command UCI command.
/// @return List of tokens.
vector<string> get_tokens(const string& command);

/// @brief Encodes pure coordinate notation of a move.
/// @param move_str pure coordinate notation of a move.
/// @return Encoded move.
Move parse_move_string(const string move_str);

/// @brief Handles the UCI 'go' command.
/// @param command UCI 'go' command line.
void handle_go(const string& command);

/// @brief Initialises engine and all dependencies.
void setup_engine();

/// @brief Safely frees memory.
void clean();

#endif