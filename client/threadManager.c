
/*
threadManager.c
----------------------------------------------------------------------------
All thread related actions
*/

//-------------------------------------------------------------//
// --------------------------INCLUDE-------------------------- //
//-------------------------------------------------------------//

#include "threadManager.h"

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


//---------------------------------------------------------------//
// ------------------------IMPLEMENTAIONS------------------------//
//---------------------------------------------------------------//


void WaitError(DWORD wait_res)
{
    switch (wait_res)
    {
        case WAIT_ABANDONED:
        {
            printf("there was an abandoned sync element\n");
            break;
        }
        case WAIT_TIMEOUT:
        {
            printf("there was a sync element for which we waited too long\n");
            break;
        }
        case WAIT_FAILED:
        {
            printf("there was a failed sync element\n");
            break;
        }
        default:
        {
            printf("there was an unrecognized sync element issue\n");
            break;
        }
    }
}


int WhatWasReceived(char received_string[])
{
    if (0 == strncmp(received_string, "SERVER_MAIN_MENU", strlen("SERVER_MAIN_MENU") - 2)) return SERVER_MAIN_MENU;
    if (0 == strncmp(received_string, "SERVER_APPROVED", strlen("SERVER_APPROVED") - 2)) return SERVER_APPROVED;
    if (0 == strncmp(received_string, "SERVER_INVITE", strlen("SERVER_INVITE") - 2)) return SERVER_INVITE;
    if (0 == strncmp(received_string, "SERVER_SETUP_REQUEST", strlen("SERVER_SETUP_REQUEST") - 2)) return SERVER_SETUP_REQUEST;
    if (0 == strncmp(received_string, "SERVER_PLAYER_MOVE_REQUEST", strlen("SERVER_PLAYER_MOVE_REQUEST") - 2)) return SERVER_PLAYER_MOVE_REQUEST;
    if (0 == strncmp(received_string, "SERVER_GAME_RESULTS", strlen("SERVER_GAME_RESULTS") - 2)) return SERVER_GAME_RESULTS;
    if (0 == strncmp(received_string, "SERVER_WIN", strlen("SERVER_WIN") - 2)) return SERVER_WIN;
    if (0 == strncmp(received_string, "SERVER_DRAW", strlen("SERVER_DRAW") - 2)) return SERVER_DRAW;
    if (0 == strncmp(received_string, "SERVER_NO_OPPONENTS", strlen("SERVER_NO_OPPONENTS") - 2)) return SERVER_NO_OPPONENTS;
    if (0 == strncmp(received_string, "SERVER_OPPONENT_QUIT", strlen("SERVER_OPPONENT_QUIT") - 2)) return SERVER_OPPONENT_QUIT;
    if (0 == strncmp(received_string, "SERVER_DENIED", strlen("SERVER_DENIED") - 2)) return SERVER_DENIED;

// Upon failure
return STATUS_CODE_FAILURE;
}


void OpponentNameLenInBytes(char received_string[], int* p_opponent_name_len)
{
    int i = 13;
    while (received_string[i] != '\n')
    {
        (*p_opponent_name_len)++;
        i++;
    }
    return;
}


void BreakDownGameResultsString(char received_string[], char bulls[], char cows[], char opponent_username[], char opponent_guess[], int opponent_name_len)
{
    int i = 0,
        bulls_index = 20,
        cows_index = 22,
        opponent_name_start_index = 24,
        opponent_name_stop_index = opponent_name_start_index + opponent_name_len,
        opponent_guess_start_index = opponent_name_stop_index + 1;
    bulls[0] = received_string[bulls_index];
    bulls[1] = '\0';
    cows[0] = received_string[cows_index];
    cows[1] = '\0';
    for (i = 0; i < (opponent_name_len - 1); i++) opponent_username[i] = received_string[opponent_name_start_index + i];
    opponent_username[i] = '\0';
    for (i = 0; i < 4; i++) opponent_guess[i] = received_string[opponent_guess_start_index + i - 1];
    opponent_guess[i] = '\0';
    return;
}


void GetWinnersNameAndOpponentsGuess(char received_string[], char winners_name[], char opponent_guess[])
{
    int i = 0, winners_name_len = 0;

    while (received_string[winners_name_len + 11] != ';')
    {
        winners_name[winners_name_len] = received_string[winners_name_len + 11];
        winners_name_len++;
    }
    winners_name[winners_name_len] = '\0';

    for (i = 0; i < 4; i++) opponent_guess[i] = received_string[i + 12 + winners_name_len];
    opponent_guess[i] = '\0';
}


char* GetStringFromClient(char user_input[])
{
    gets_s(user_input, 21); //Reading a string from the keyboard
    return user_input;
}


void DefineStringToSend(int* p_game_state, char user_input[], char send_string[])
{
    char    client_request[] = "CLIENT_REQUEST:",
        client_versus_str[] = "CLIENT_VERSUS\n",
        client_setup[] = "CLIENT_SETUP:", //19 max
        client_player_move[] = "CLIENT_PLAYER_MOVE:", //25 max
        client_disconnect_str[] = "CLIENT_DISCONNECT\n";

    int i = 0,
        client_request_i = strlen(client_request),
        client_setup_i = strlen(client_setup),
        client_player_move_i = strlen(client_player_move);

    GetStringFromClient(user_input);

    switch (*p_game_state)
    {
        case SERVER_APPROVED:
        {
            for (i = 0; i < client_request_i; i++) send_string[i] = client_request[i];
            i = 0;
            while (user_input[i] != '\0')
            {
                send_string[i + client_request_i] = user_input[i];
                i++;
            }
            send_string[i + client_request_i] = '\n';
            send_string[i + client_request_i + 1] = '\0';
            break;
        }
        case SERVER_MAIN_MENU:
        {
            while (atoi(user_input) < 1 || atoi(user_input) > 2)
            {
                printf("You need to choose either '1' or '2'. Other options are unacceptable\n");
                GetStringFromClient(user_input);
            }
                if (user_input[0] == '1') strcpy_s(send_string, MAX_BYTES_CLIENT_MIGHT_SEND, client_versus_str);
                if (user_input[0] == '2')
                {
                    strcpy_s(send_string, MAX_BYTES_CLIENT_MIGHT_SEND, client_disconnect_str);
                    *p_game_state = I_QUIT;
                }
                break;
            }
        case SERVER_SETUP_REQUEST:
        {
            i = 0;
            for (i = 0; i < client_setup_i; i++) send_string[i] = client_setup[i];
            for (i = 0; i < 4; i++) send_string[i + client_setup_i] = user_input[i];
            send_string[i + client_setup_i] = '\n';
            send_string[i + client_setup_i + 1] = '\0';
            break;
        }
        case SERVER_PLAYER_MOVE_REQUEST:
        {
            i = 0;
            for (i = 0; i < client_player_move_i; i++) send_string[i] = client_player_move[i];
            for (i = 0; i < 4; i++) send_string[i + client_player_move_i] = user_input[i];
            send_string[i + client_player_move_i] = '\n';
            send_string[i + client_player_move_i + 1] = '\0';
            break;
        }

    }
    return;
}


int GameState(SOCKET* m_socket, SOCKADDR_IN* p_clientService, int* p_game_state, char user_input[], char send_string[], char received_string[], char server_ip[], char server_port[], int* p_opponent_name_len, char winners_name[], char opponent_username[], char bulls[], char cows[], char opponent_guess[])
{
    int res = 0;
    switch (*p_game_state)
    {
    case I_START:
    {
        printf("Connected to server on %s:%s\n", server_ip, server_port);
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = WhatWasReceived(received_string);
        break;
    }
    case SERVER_APPROVED:
    {
        printf("Type in your name:\n");
        DefineStringToSend(p_game_state, user_input, send_string);
        if (STATUS_CODE_FAILURE == (SendData(*m_socket, send_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        *p_game_state = WhatWasReceived(received_string);
        printf("\n");
        break;
    }
    case SERVER_MAIN_MENU:
    {
        *p_opponent_name_len = 0;
        printf("Choose what to do next:\n1. Play against another client\n2. Quit\n");
        DefineStringToSend(p_game_state, user_input, send_string);
        if (STATUS_CODE_FAILURE == (SendData(*m_socket, send_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        if (*p_game_state == I_QUIT) break;
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = WhatWasReceived(received_string);
        break;
    }
    case SERVER_NO_OPPONENTS:
    {
        printf("There are currently no opponents available\n");
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = SERVER_MAIN_MENU;
        break;
    }
    case SERVER_INVITE:
    {
        OpponentNameLenInBytes(received_string, p_opponent_name_len);
        printf("Game is on!\n");
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = WhatWasReceived(received_string);
        break;
    }
    case SERVER_SETUP_REQUEST:
    {
        printf("Choose your 4 digits:\n");
        DefineStringToSend(p_game_state, user_input, send_string);
        if (STATUS_CODE_FAILURE == (SendData(*m_socket, send_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = WhatWasReceived(received_string);
        break;
    }
    case SERVER_PLAYER_MOVE_REQUEST:
    {
        printf("Choose your guess:\n");
        DefineStringToSend(p_game_state, user_input, send_string);
        if (STATUS_CODE_FAILURE == (SendData(*m_socket, send_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = WhatWasReceived(received_string);
        break;
    }
    case SERVER_GAME_RESULTS:
    {
        BreakDownGameResultsString(received_string, bulls, cows, opponent_username, opponent_guess, *p_opponent_name_len);
        printf("Bulls: %s\nCows: %s\n%s played: %s\n", bulls, cows, opponent_username, opponent_guess);
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = WhatWasReceived(received_string);
        break;
    }
    case SERVER_WIN:
    {
        GetWinnersNameAndOpponentsGuess(received_string, winners_name, opponent_guess);
        printf("%s won!\nopponents number was %s\n", winners_name, opponent_guess);
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = WhatWasReceived(received_string);
        break;
    }
    case SERVER_DRAW:
    {
        printf("It’s a tie\n");
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = WhatWasReceived(received_string);
        break;
    }
    case SERVER_OPPONENT_QUIT:
    {
        printf("Opponent quit.\n");
        if (STATUS_CODE_FAILURE == (RecvData(*m_socket, received_string)))
        {
            *p_game_state = I_FAIL;
            break;
        }
        printf("\n");
        *p_game_state = WhatWasReceived(received_string);
        break;
    }
    case SERVER_DENIED:
    {
        if (closesocket(*m_socket) == SOCKET_ERROR)
        {
            printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
            *p_game_state = I_QUIT;
        }
        printf("Server on %s:%s denied the connection request.\nChoose what to do next:\n1. Try to reconnect\n2. Exit\n", server_ip, server_port);
        GetStringFromClient(user_input);
        while (atoi(user_input) < 1 || atoi(user_input) > 2)
        {
            printf("You need to choose either '1' or '2'. Other options are unacceptable\n");
            GetStringFromClient(user_input);
        }
        if (user_input[0] == '1')
        {
            if ((*m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
            {
                printf("Error at socket(): %ld\n", WSAGetLastError());
                WSACleanup();
                return STATUS_CODE_FAILURE;
            }
            if (connect(*m_socket, (SOCKADDR*)p_clientService, sizeof(*p_clientService)) == SOCKET_ERROR) *p_game_state = I_FAIL;
            else *p_game_state = I_START;
        }
        if (user_input[0] == '2')
        {
            *p_game_state = I_QUIT;
        }
        break;
    }
    case I_FAIL:
    {
        if (closesocket(*m_socket) == SOCKET_ERROR)
        {
            printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
            *p_game_state = I_QUIT;
        }
        printf("Failed connecting to server on %s:%s.\nChoose what to do next:\n1. Try to reconnect\n2. Exit\n", server_ip, server_port);
        GetStringFromClient(user_input);
        while (atoi(user_input) < 1 || atoi(user_input) > 2)
        {
            printf("You need to choose either '1' or '2'. Other options are unacceptable\n");
            GetStringFromClient(user_input);
        }
        if (user_input[0] == '1')
        {
            if ((*m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
            {
                printf("Error at socket(): %ld\n", WSAGetLastError());
                WSACleanup();
                return STATUS_CODE_FAILURE;
            }
            if (connect(*m_socket, (SOCKADDR*)p_clientService, sizeof(*p_clientService)) == SOCKET_ERROR) *p_game_state = I_FAIL;
            else *p_game_state = I_START;
        }
        if (user_input[0] == '2')
        {
            *p_game_state = I_QUIT;
        }
        break;
    }
    case I_QUIT:
    {
        return SUCCESS_CODE;
    }
    }
    return SUCCESS_CODE;
}
