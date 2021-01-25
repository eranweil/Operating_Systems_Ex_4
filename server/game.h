#pragma once

/*
game.h
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

#include "threadManager.h"
#include "HardCodedData.h"

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