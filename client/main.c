/*
main.c
----------------------------------------------------------------------------
AUTHORS - Itamar Eyal 302309539 and Eran Weil 204051353

PROJECT - Factori

DESCRIPTION - This program can find the prime number building blocks for all of the numbers given in a task file by the user


    consists of 4 main modules:
        fileManager - handles files and cmd line args
        threadManager - creates and handles threads
        Lock - A module for all of the lock specific actions, comprising mostly of sync object actions
        Queue - A module for all of the queue specific actions, comprising mostly of linked list actions
*/

//-------------------------------------------------------------//
// ----------------------LIBRARY INCLUDES--------------------- //
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

#include "HardCodedData.h"
#include "threadManager.h"
#include "fileManager.h"


void GameState(SOCKET* m_socket, SOCKADDR_IN* p_clientService, int* p_game_state, char user_input[], char send_string[], char received_string[], char server_ip[], char server_port[], int* p_opponent_name_len, char winners_name[], char opponent_username[], char bulls[], char cows[], char opponent_guess[])
{
    switch (*p_game_state)
    {
        case I_START:
        {
            printf("Connected to server on %s:%s\nType in your name:\n", server_ip, server_port);
            DefineStringToSend(p_game_state, user_input, send_string);
            SendData(*m_socket, send_string);
            RecvData(*m_socket, received_string);
            *p_game_state = WhatWasReceived(received_string);
            break;
        }
        case SERVER_APPROVED:
        {
            RecvData(*m_socket, received_string);
            *p_game_state = SERVER_MAIN_MENU;
            break;
        }
        case SERVER_MAIN_MENU:
        {
            printf("Choose what to do next:\n1. Play against another client\n2. Quit\n");
            DefineStringToSend(p_game_state, user_input, send_string);
            SendData(*m_socket, send_string);
            RecvData(*m_socket, received_string);
            *p_game_state = WhatWasReceived(received_string);
            break;        
        }
        case SERVER_NO_OPPONENTS:
        {
            printf("There are currently no opponents available\n");
            RecvData(*m_socket, received_string);
            *p_game_state = SERVER_MAIN_MENU;
            break;
        }
        case SERVER_INVITE:
        {
            OpponentNameLenInBytes(received_string, p_opponent_name_len);
            printf("Game is on!\n");
            RecvData(*m_socket, received_string);
            *p_game_state = WhatWasReceived(received_string);
            break;        
        }
        case SERVER_SETUP_REQUEST:
        {
            printf("Choose your 4 digits:\n");
            DefineStringToSend(p_game_state, user_input, send_string);
            SendData(*m_socket, send_string);
            RecvData(*m_socket, received_string);
            *p_game_state = WhatWasReceived(received_string);
            break;        
        }
        case SERVER_PLAYER_MOVE_REQUEST:
        {
            printf("Choose your guess:\n");
            DefineStringToSend(p_game_state, user_input, send_string);
            SendData(*m_socket, send_string);
            RecvData(*m_socket, received_string);
            *p_game_state = WhatWasReceived(received_string);
            break;        
        }
        case SERVER_GAME_RESULTS:
        {
            BreakDownGameResultsString(received_string, bulls, cows, opponent_username, opponent_guess, *p_opponent_name_len);
            printf("Bulls: %s\nCows: %s\n%s played: %s\n", bulls, cows, opponent_username, opponent_guess);
            RecvData(*m_socket, received_string);
            *p_game_state = WhatWasReceived(received_string);
            break;        
        }
        case SERVER_WIN:
        {
            GetWinnersNameAndOpponentsGuess(received_string, winners_name, opponent_guess);
            printf("%s won!\nopponents number was %s\n", winners_name, opponent_guess);
            RecvData(*m_socket, received_string);
            *p_game_state = WhatWasReceived(received_string);
            break;        
        }
        case SERVER_DRAW:
        {
            printf("It’s a tie");
            RecvData(*m_socket, received_string);
            *p_game_state = WhatWasReceived(received_string);
            break;        
        }
        case SERVER_OPPONENT_QUIT:
        {
            printf("Opponent quit.");
            RecvData(*m_socket, received_string);                   //What to do? restart game?
            *p_game_state = WhatWasReceived(received_string);
            break;        
        }
        case SERVER_DENIED:
        {
            closesocket(*m_socket);
            printf("Server on %s:%s denied the connection request.\nChoose what to do next:\n1. Try to reconnect\n2. Exit\n", server_ip, server_port);
            GetStringFromClient(user_input);
            while (atoi(user_input) < 1 || atoi(user_input) > 2)
            {
                printf("You need to choose either '1' or '2'. Other options are unacceptable\n");
                GetStringFromClient(user_input);
            }
            if (user_input[0] = '1')
            {
                if (connect(m_socket, (SOCKADDR*)p_clientService, sizeof(*p_clientService)) == SOCKET_ERROR) *p_game_state = I_FAIL;
                else *p_game_state = I_START;
            }
            if (user_input[0] = '2')
            {
                *p_game_state = I_QUIT;
            }
            break;       
        }
        case I_FAIL:
        {
            while (connect(*m_socket, (SOCKADDR*)p_clientService, sizeof(*p_clientService)) == SOCKET_ERROR)
            {
                printf("Failed connecting to server on %s:%s.\nChoose what to do next:\n1. Try to reconnect\n2. Exit\n", server_ip, server_port);
                GetStringFromClient(user_input);

                while (atoi(user_input) < 1 || atoi(user_input) > 2)
                {
                    printf("You need to choose either '1' or '2'. Other options are unacceptable\n");
                    GetStringFromClient(user_input);
                }
                if (user_input[0] = '1') continue;
                if (user_input[0] = '2')
                {
                    *p_game_state = I_QUIT;
                    break;
                }
            }
            *p_game_state = I_START;
            break;
        }
    }
    return;
}


int main(int argc, char** argv[])
{
    char        user_input[21] = "",
                opponent_username[21] = "",
                winners_name[21] = "",
                opponent_guess[5] = "",
                bulls[2] = "",
                cows[2] = "",
                send_string[37] = "",
                received_string[MAX_BYTES_SERVER_MIGHT_SEND] = "";

    char*       server_ip[16]= "",
                server_port[5] = "",
                client_name[21] = "";
                strcpy(server_ip, argv[1]);
                strcpy(server_port, argv[2]);
                strcpy(client_name, argv[3]);

    int		    server_port_num = atoi(server_port),
                game_state = I_QUIT,
                opponent_name_len = 0;

    int*        p_game_state = &game_state,
                p_opponent_name_len = opponent_name_len;

	SOCKET	    m_socket;
	SOCKADDR_IN clientService;

	HANDLE      hThread[2];
    HANDLE      mutex_file, sephamore_file, turnstile, queue_lock, start_line_sephamore;

    LOCK        lock;
    LOCK*       p_lock = &lock;

    // Check args given
    if (STATUS_CODE_FAILURE == check_arguments(argc, argv)) return ERROR_CODE_ARGS;

    // Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (StartupRes != NO_ERROR)
    {
        printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
        // Tell the user that we could not find a usable WinSock DLL.                                  
        return;
    }

	// Create a socket and check for errors to ensure that the socket is a valid socket.
	if ((m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) 
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	// Connect to a server.

	// Create a sockaddr_in object clientService and set  values.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(server_ip); //Setting the IP address to connect to
	clientService.sin_port = htons(server_port_num); //Setting the port to connect to.

	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
    
    game_state = I_START;

    if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) game_state = I_FAIL;
    
    while (game_state != I_QUIT)
    {
        GameState(&m_socket, &clientService, p_game_state, user_input, send_string, received_string, server_ip, server_port, p_opponent_name_len, winners_name, opponent_username, bulls, cows, opponent_guess);
    }

    closesocket(m_socket);

    return SUCCESS_CODE;
}