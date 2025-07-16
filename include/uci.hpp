#ifndef UCI_HPP_INCLUDE
#define UCI_HPP_INCLUDE

#include <string>
#include <vector>
#include <cstdint>

using Move = uint16_t;

/// @brief UCI command handler.
/// @param command UCI command.
/// @return true if received 'quit' command.
bool handle_command(const std::string& command);

/// @brief Prints engine info to std::cout.
void send_info();

/// @brief Tokenises a UCI command.
/// @param command UCI command.
/// @return List of tokens.
std::vector<std::string> get_tokens(const std::string& command);

/// @brief Encodes pure coordinate notation of a move.
/// @param move_str pure coordinate notation of a move.
/// @return Encoded move.
Move parse_move_string(const std::string move_str);

/// @brief Handles the UCI 'go' command.
/// @param command UCI 'go' command line.
void handle_go(const std::string& command);

/// @brief Initialises engine and all dependencies.
void setup_engine();

/// @brief Safely frees memory.
void clean();

#endif