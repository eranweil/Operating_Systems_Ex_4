


client_number[5]
opponent_number[5]
client_guess[5]
opponent_guess[5]
client_name[21]
opponent_name[21]
server_game_results_str[66] = "SERVER_GAME_RESULTS:num_of_bulls;num_of_cows;opponents_name;opponents_guess\n";
server_win_str[38] = "SERVER_WIN:winners_name;opponents_guess\n"
p_game_result

// Game result codes
#define GAME_CONTINUES                          0
#define GAME_WON                                1
#define GAME_DRAW                               2


void GetGameResults(char client_number[], char opponent_number[], char client_guess[], char opponent_guess[], char client_name[], char opponent_name[], char server_game_results_str[], char server_win_str[], int* p_game_result)
{
	// The function belongs to the thread working with the client
	// The the function calculates if somebody won, if it is a draw or if the game continues
	// If there is a win:
		// update server_win_str[] with the correct name of the winner, which is either client or opponent
		// update &p_game_result as GAME_WON
	// If there is a draw:
		// update &p_game_result as GAME_DRAW
	// If neither, the game continues
		// update server_game_results_str[] with the correct number of bulls, cows, opponents_name and opponents_guess
		// &p_game_result is already GAME_CONTINUES
}