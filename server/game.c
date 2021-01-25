
/*
game.c
----------------------------------------------------------------------------
bulls & cows functions
*/

//-------------------------------------------------------------//
// ----------------------PROJECT INCLUDES--------------------- //
//-------------------------------------------------------------//

#include "game.h"

//---------------------------------------------------------------//
// -------------------------DECLARATIONS------------------------ //
//---------------------------------------------------------------//

/*--------------------------------------------------------------------------------------------
DESCRIPTION - takes guesses and numbers of the client and it's opponent. declares a winner/draw if there is one. otherwise let's the
			  client get his results.

PARAMETERS -    client_number[]
				opponent_number[]
				client_guess[]
				opponent_guess[]
				client_name[]
				opponent_name[]
				server_game_results_str[] - holds bulls,cows, opponent name and guess
				server_win_str[] - changes to winners name if he wins
				p_game_result - holding the result of the round

RETURN - void
	--------------------------------------------------------------------------------------------*/
void GetGameResults(char client_number[], char opponent_number[], char client_guess[], char opponent_guess[],
	char client_name[], char opponent_name[], char server_game_results_str[], char server_win_str[], int* p_game_result);


/*--------------------------------------------------------------------------------------------
DESCRIPTION - takes a guess and a target number, returns a string with the result. win is 'BBBB'

PARAMETERS -    char player_guess[]
				char target_number[]

RETURN - 4 char array with B/C/M for bull/cow/miss.
	--------------------------------------------------------------------------------------------*/
void CheckGuess(char player_guess[], char target_number[], char* res);


/*--------------------------------------------------------------------------------------------
DESCRIPTION - takes an array holding results by chars and counts occurences of A.

PARAMETERS -    player_res[]

RETURN - pointer to the number as char
	--------------------------------------------------------------------------------------------*/
void GetBullsOrCows(char player_res[], char A, char dest[]);


/*--------------------------------------------------------------------------------------------
DESCRIPTION - clears \r\n from array

PARAMETERS -    raw[]

RETURN - void
	--------------------------------------------------------------------------------------------*/
void CleanName(char raw[]);



//---------------------------------------------------------------//
// ----------------------IMPLEMENTATIONS------------------------ //
//---------------------------------------------------------------//


void GetGameResults(char client_number[], char opponent_number[], char client_guess[], char opponent_guess[],
	char client_name[], char opponent_name[], char server_game_results_str[], char server_win_str[], int* p_game_result)
{

	int client_win = 0; 
	int opponent_win = 0;
	int game = 0;
	unsigned int i;
	char victory[5] = "BBBB";
	char server_game_results[] = "SERVER_GAME_RESULTS:";
	char server_win[] = "SERVER_WIN:";

	CleanName(client_name);
	CleanName(client_number);
	CleanName(client_guess);

	char client_res[5] = "MMMM";
	CheckGuess(client_guess, opponent_number, client_res);
	char opponenet_res[5] = "MMMM";
	CheckGuess(opponent_guess, client_number, opponenet_res);

	client_win = strcmp(client_res, victory) == 0;
	opponent_win = strcmp(opponenet_res, victory) == 0;

	char client_bulls[2] = "0";
	GetBullsOrCows(client_res, 'B', client_bulls);
	char client_cows[2] = "0";
	GetBullsOrCows(client_res, 'C', client_cows);
	char oponnent_bulls[2] = "0";
	GetBullsOrCows(opponenet_res, 'B', oponnent_bulls);
	char oponnent_cows[2] = "0";
	GetBullsOrCows(opponenet_res, 'C', oponnent_cows);

	game = client_win + opponent_win;

	switch (game) {
		case 0: // game continues
			*p_game_result = GAME_CONTINUES;
			// updating server_game_results_str array
			for (i = 0; i < strlen(server_game_results); i++) server_game_results_str[i] = server_game_results[i];
			for (i = 0; i < strlen(client_bulls); i++) server_game_results_str[i + strlen(server_game_results)] = client_bulls[i];
			server_game_results_str[i + strlen(server_game_results)] = ';';
			for (i = 0; i < strlen(client_cows); i++) server_game_results_str[i + 1 + strlen(server_game_results) + strlen(client_bulls)] = client_cows[i];
			server_game_results_str[i + 1 + strlen(server_game_results) + strlen(client_bulls)] = ';';
			for (i = 0; i < strlen(opponent_name); i++) server_game_results_str[i + 2 + strlen(server_game_results) + strlen(client_bulls) + strlen(client_cows)] = opponent_name[i];
			server_game_results_str[i + 2 + strlen(server_game_results) + strlen(client_bulls) + strlen(client_cows)] = ';';
			for (i = 0; i < strlen(opponent_guess); i++) server_game_results_str[i + 3 + strlen(server_game_results) + strlen(client_bulls) + strlen(client_cows) + strlen(opponent_name)] = opponent_guess[i];
			server_game_results_str[i + 3 + strlen(server_game_results) + strlen(client_bulls) + strlen(client_cows) + strlen(opponent_name)] = '\n';
			server_game_results_str[i + 4 + strlen(server_game_results) + strlen(client_bulls) + strlen(client_cows) + strlen(opponent_name)] = '\0';
			break;

		case 1: // we have a winner!
			if (client_win) {
				for (i = 0; i < strlen(server_win); i++) server_win_str[i] = server_win[i];
				for (i = 0; i < strlen(client_name); i++) server_win_str[i + strlen(server_win)] = client_name[i];
				server_win_str[i + strlen(server_win)] = '\n';
				server_win_str[i + 1 + strlen(server_win)] = '\0';
			}
			else { // opponent wins
				for (i = 0; i < strlen(server_win); i++) server_win_str[i] = server_win[i];
				for (i = 0; i < strlen(opponent_name); i++) server_win_str[i + strlen(server_win)] = opponent_name[i];
				server_win_str[i + strlen(server_win)] = '\n';
				server_win_str[i + 1 + strlen(server_win)] = '\0';
			}
			*p_game_result = GAME_WON;
			break;

		case 2: // draw
			*p_game_result = GAME_DRAW;
			break;
	}
}


void CheckGuess(char player_guess[], char target_number[], char* res) {

	int i, j;

	for (i = 0; i < 4; i++) {
		if (target_number[i] == player_guess[i]) {
			res[i] = 'B'; // bull, move on to next char in guess
			continue;
		}
		for (j = 0; j < 4; j++) {
			if ((player_guess[i] == target_number[j]) && (i!=j))
				// matching char, wrong index -> cow
				res[i] = 'C'; // cow
		}
	}
	res[i++] = '\0';
}


void GetBullsOrCows(char player_res[], char A, char dest[]) {
	int i;
	int count = 0;

	for (i = 0; i < 4; i++) {
		if (player_res[i] == A)
			count++;
	}
	dest[0] = (count + '0');
	dest[1] = '\0';
}


void CleanName(char raw[]) {
	for (int i = 0, j = 0; raw[i] != '\0'; i++)
		if ((raw[i] != '\r') && (raw[i] != '\n'))
			raw[j++] = raw[i];
		else
			raw[j++] = '\0';
}
