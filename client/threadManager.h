#pragma once
/*
threadManager.h
----------------------------------------------------------------------------
All thread related actions header file
*/

//-------------------------------------------------------------//
// --------------------------INCLUDE-------------------------- //
//-------------------------------------------------------------//

#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>

//-------------------------------------------------------------//
// ----------------------PROJECT INCLUDES--------------------- //
//-------------------------------------------------------------//

#include "SocketSendRecvTools.h"
#include "HardCodedData.h"


//---------------------------------------------------------------//
// -------------------------DECLARATIONS------------------------ //
//---------------------------------------------------------------//

/*--------------------------------------------------------------------------------------------
DESCRIPTION - check what the client sent

PARAMETERS - client sent string

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int WhatWasReceived(char* AcceptedStr);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Gets opponent name length in bytes and saves it to the address of a pointer

PARAMETERS - string to calculate bytes and pointer to address where we wish to save the length to

RETURN - void
    --------------------------------------------------------------------------------------------*/
void OpponentNameLenInBytes(char received_string[], int* p_opponent_name_len);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - function to get the results of the current game stage and saves it to all the buffers given

PARAMETERS - the string received, the length of the opponent name and buffers to save the number of cows, bulls, opponent guess and opponent name

RETURN - void
    --------------------------------------------------------------------------------------------*/
void BreakDownGameResultsString(char received_string[], char bulls[], char cows[], char opponent_username[], char opponent_guess[], int opponent_name_len);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - function to get the winners name and opponent number

PARAMETERS - the string received, and buffers to save the winners name and opponent number

RETURN - void
    --------------------------------------------------------------------------------------------*/
void GetWinnersNameAndOpponentsGuess(char received_string[], char winners_name[], char opponent_guess[]);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - a simple function to get input from the client
    --------------------------------------------------------------------------------------------*/
char* GetStringFromClient(char user_input[]);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - A function which creates the string to send based on game stage and user input

PARAMETERS - game stage, user input and buffer to which we write the string to send

RETURN - void
    --------------------------------------------------------------------------------------------*/
void DefineStringToSend(int* p_game_state, char user_input[], char send_string[]);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - The main client function. calculates what to do at wach state based on the server data sent and client input

PARAMETERS - All of the necessary buffers to save data to for read and write, as well as the socket data to communicate with the server

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int GameState(SOCKET* m_socket, SOCKADDR_IN* p_clientService, int* p_game_state, char user_input[], char send_string[], char received_string[], char server_ip[], char server_port[], int* p_opponent_name_len, char winners_name[], char opponent_username[], char bulls[], char cows[], char opponent_guess[]);

