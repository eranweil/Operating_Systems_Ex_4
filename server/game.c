//client_number[5]
//opponent_number[5]
//client_guess[5]
//opponent_guess[5]
//client_name[21] //that is the max, 20 charachters and a NULL sign. Could be the name is less
//opponent_name[21] //that is the max, 20 charachters and a NULL sign. Could be the name is less
//server_game_results_str[66] = "SERVER_GAME_RESULTS:num_of_bulls;num_of_cows;opponents_name;opponents_guess\n";
//server_win_str[38] = "SERVER_WIN:winners_name;opponents_guess\n"
//p_game_result


/*
game.c
----------------------------------------------------------------------------
bulls & cows functions
*/

//-------------------------------------------------------------//
// --------------------------INCLUDE-------------------------- //
//-------------------------------------------------------------//

#include <stdio.h>
#include <stdlib.h>

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
char* CheckGuess(char player_guess[], char target_number[]);


/*--------------------------------------------------------------------------------------------
DESCRIPTION - takes an array holding results by chars and counts occurences of A.

PARAMETERS -    player_res[]

RETURN - pointer to the number as char
	--------------------------------------------------------------------------------------------*/
char* GetBullsOrCows(char player_res[], char A);


//---------------------------------------------------------------//
// ----------------------IMPLEMENTATIONS------------------------ //
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
	char client_name[], char opponent_name[], char server_game_results_str[], char server_win_str[], int* p_game_result)
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
	int client_win = 0; 
	int opponent_win = 0;
	int game = 0;
	char victory[5] = "BBBB\0";

	char client_res[] = CheckGuess(client_guess, opponent_number);
	char opponenet_res[] = CheckGuess(opponent_guess, client_number);


	client_win = strcmp(client_res, victory) == 0;
	opponent_win = strcmp(opponenet_res, victory) == 0;

	char *client_bulls = GetBullsOrCows(client_res, 'B');
	char *client_cows = GetBullsOrCows(client_res, 'C');
	char *oponnent_bulls = GetBullsOrCows(opponenet_res, 'B');
	char *oponnent_cows = GetBullsOrCows(opponenet_res, 'C');

	game = client_win + opponent_win;

	switch (game) {
		case 0: // game continues
	
			// updating server_game_results_str array
			sprintf(server_game_results_str, "SERVER_GAME_RESULTS:");
			strcat(server_game_results_str, client_bulls);
			strcat(server_game_results_str, ";");
			strcat(server_game_results_str, client_cows);
			strcat(server_game_results_str, ";");
			strcat(server_game_results_str, opponent_name);
			strcat(server_game_results_str, ";");
			strcat(server_game_results_str, opponent_guess);
			strcat(server_game_results_str, "\0");
			break;

		case 1: // we have a winner!
			if (client_win) {
				sprintf(server_win_str, client_name);
			}
			else { // opponent wins
				sprintf(server_win_str, opponent_name);
			}
			strcat(server_win_str, "\0");
			p_game_result = GAME_WON;
			break;

		case 2: // draw
			p_game_result = GAME_DRAW;
			break;
	}
}


/*--------------------------------------------------------------------------------------------
DESCRIPTION - takes a guess and a target number, returns a string with the result. win is 'BBBB'

PARAMETERS -    char player_guess[]
				char target_number[]

RETURN - 4 char array with B/C/M for bull/cow/miss. 
	--------------------------------------------------------------------------------------------*/
char* CheckGuess(char player_guess[], char target_number[]) {
	char res[5] = "MMMM\0";
	int i, j;

	for (i = 0; i < 4; i++) {
		if (strcmp(target_number[i], player_guess[i]) == 0) {
			*(res + i) = 'B'; // bull, move on to next char in guess
			continue;
		}
		for (j = 0; j < 4; j++) {
			if (strcmp(player_guess[i], target_number[j]) == 0 && i!=j)
				// matching char, wrong index -> cow
				*(res + i) = 'C'; // cow
		}
	}
	return *res;
}


/*--------------------------------------------------------------------------------------------
DESCRIPTION - takes an array holding results by chars and counts occurences of A.

PARAMETERS -    player_res[]

RETURN - pointer to the number as char
	--------------------------------------------------------------------------------------------*/
char *GetBullsOrCows(char player_res[], char A) {
	int i;
	int count = 0;
	char* res[2];
	for (i = 0; i < 4; i++) {
		if (strcmp(player_res, A) == 0)
			count++;
	}
	sprintf(res, "%d", count);
	return res;
}
