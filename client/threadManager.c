
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
DESCRIPTION - A debug function to print out the wait error on a sync element

PARAMETERS -    wait_res - the result of waiting on the sync element
                thread_num - the number of the thread, used for print
                number_of_tasks - number of tasks to be pushed into the queue
                priority_file_name - The name of the priority file for the WINAPI read function

RETURN - void
    --------------------------------------------------------------------------------------------*/
void WaitError(DWORD wait_res);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Calls a wait for multiple objects on an array with all of the running threads

PARAMETERS - p_threads - an array of thread handles
             number_of_threads - the number of threads to wait for

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int wait_for_threads_execution_and_free(HANDLE* p_threads, int number_of_threads);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - the mother function which dispatches the threads and waits for them to finish their good work.

PARAMETERS - p_threads - a pointer to an array of handles holding all of the thread handles
             p_lock - a pointer to the joint lock element
             p_queue - a pointer to the joint queue element
             number_of_threads - the number of threads specified by the user
             p_number_of_tasks - a pointer to an integer with the number of tasks left. each thread is responsible to update it
             start_line_sephamore - this is a joint semaphore used to send all of the threads on their way simultaneously
             tasks_file_name - the name of the tasks file

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int dispatch_threads(HANDLE hThread[], SOCKET* m_socket, LOCK* p_lock);


int WhatWasReceived(char* AcceptedStr);


void OpponentNameLenInBytes(char received_string[], int* p_opponent_name_len);


void BreakDownGameResultsString(char received_string[], char bulls[], char cows[], char opponent_username[], char opponent_guess[], int opponent_name_len);


void GetWinnersNameAndOpponentsGuess(char received_string[], char winners_name[], char opponent_guess[]);


char* GetStringFromClient(char user_input[]);


void DefineStringToSend(int game_state, char user_input[], char send_string[]);


/*--------------------------------------------------------------------------------------------
DESCRIPTION - Function every new thread is called to. reads a task from the task file, breaks into primes and prints the correct string to the tasks file. uses a lock regiment as specified

PARAMETERS - lpParam: holds the data structure of pData for that thread

RETURN - signal exit code.
    --------------------------------------------------------------------------------------------*/
DWORD WINAPI RecvData(SOCKET* m_socket, char received_string[]);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Function every new thread is called to. reads a task from the task file, breaks into primes and prints the correct string to the tasks file. uses a lock regiment as specified

PARAMETERS - lpParam: holds the data structure of pData for that thread

RETURN - signal exit code.
    --------------------------------------------------------------------------------------------*/
DWORD WINAPI SendData(SOCKET* m_socket, char send_string[]);

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


int wait_for_threads_execution_and_free(HANDLE hThread[], SOCKET* m_socket)
{
    int i = 0;
    DWORD dwEvent;

    dwEvent = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

    if (WAIT_OBJECT_0 == dwEvent)
    {
        // Free the threads which were dispatched
        for (i = 0; i < 2; i++)
        {
            CloseHandle(hThread[i]);
            printf("freed Thread number: %d\n", i + 1);
        }
    }

    else
    {
        // Print Error message
        WaitError(dwEvent, 0);
        // Free the threads which were dispatched, because some might have been
        for (i = 0; i < 2; i++)
        {
            if (hThread[i] != NULL)
            {
                CloseHandle(hThread[i]);
                printf("freed Thread number: %d\n", i + 1);
            }
        }
    }

    closesocket(*m_socket);
    WSACleanup();
}


int dispatch_threads(HANDLE hThread[], SOCKET* m_socket, LOCK* p_lock)
{
    int i = 0, j = 0;

    THREAD Data;
    THREAD* pData = &Data;
    DWORD dwThreadId_receive;
    DWORD dwThreadId_send;

    pData->p_lock = p_lock;
    //pData->start_line_sephamore = p_start_line_sephamore;
    pData->m_socket = m_socket;
    *pData->receive_thread_result = -1;

    hThread[0] = CreateThread(
        NULL,
        0,
        SendDataThread,
        pData,
        0,
        &dwThreadId_send
    );

    //In case of error
    if (NULL == hThread[0])
    {
        printf("Brutally terminating the program due to thread dispatch issues");
        CloseHandle(hThread[0]);
        return STATUS_CODE_FAILURE;
    }
    //Else
    printf("dispatched sending Thread\n");

    hThread[1] = CreateThread(
        NULL,
        0,
        RecvDataThread,
        pData,
        0,
        &dwThreadId_receive
    );

    //In case of error
    if (NULL == hThread[1])
    {
        printf("Brutally terminating the program due to thread dispatch issues");
        CloseHandle(hThread[1]);
        return STATUS_CODE_FAILURE;
    }
    //Else
    printf("dispatched receiving Thread\n");

    //// Release start line Sephamore
    //if (FALSE == (ReleaseSemaphore(*p_start_line_sephamore, number_of_threads, NULL)))
    //{
    //    printf("Couldn't release start line semaphore");
    //    return STATUS_CODE_FAILURE;
    //}
    //printf("start line semaphore released\n");

    // escorting the threads for exit
    if (STATUS_CODE_FAILURE == (wait_for_threads_execution_and_free(hThread, m_socket)))
    {
        printf("There was a wait error while we waited for all of the threads");
        return STATUS_CODE_FAILURE;
    }

    // Success
    printf("freed all threads\n");
    return SUCCESS_CODE;
}


int WhatWasReceived(char received_string[])
{
    if (0 == strcmp(received_string, "SERVER_MAIN_MENU\n")) return SERVER_MAIN_MENU;
    if (0 == strcmp(received_string, "SERVER_APPROVED\n")) return SERVER_APPROVED;
    if (0 == strcmp(received_string, "SERVER_INVITE\n")) return SERVER_INVITE;
    if (0 == strcmp(received_string, "SERVER_SETUP_REQUEST\n")) return SERVER_SETUP_REQUEST;
    if (0 == strcmp(received_string, "SERVER_PLAYER_MOVE_REQUEST\n")) return SERVER_PLAYER_MOVE_REQUEST;
    if (0 == strcmp(received_string, "SERVER_GAME_RESULTS\n")) return SERVER_GAME_RESULTS;
    if (0 == strcmp(received_string, "SERVER_WIN\n")) return SERVER_WIN;
    if (0 == strcmp(received_string, "SERVER_DRAW\n")) return SERVER_DRAW;
    if (0 == strcmp(received_string, "SERVER_NO_OPPONENTS\n")) return SERVER_NO_OPPONENTS;
    if (0 == strcmp(received_string, "SERVER_OPPONENT_QUIT\n")) return SERVER_OPPONENT_QUIT;
    if (0 == strcmp(received_string, "SERVER_DENIED\n")) return SERVER_DENIED;

// Upon failure
return STATUS_CODE_FAILURE;
}


void OpponentNameLenInBytes(char received_string[], int* p_opponent_name_len)
{
    int i = 13;
    while (received_string[i] != '\n')
    {
        *p_opponent_name_len++;
        i++;
    }
    return;
}


void BreakDownGameResultsString(char received_string[], char bulls[], char cows[], char opponent_username[], char opponent_guess[], int opponent_name_len)
{
    "SERVER_GAME_RESULTS:2;2;THOMAS;1948\n";
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
    for (i = 0; i < opponent_name_len; i++) opponent_username[i] = received_string[opponent_name_start_index + i];
    opponent_username[i] = '\0';
    for (i = 0; i < 4; i++) opponent_guess[i] = received_string[opponent_guess_start_index + i];
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


void DefineStringToSend(int game_state, char user_input[], char send_string[])
{
    char    client_request_str[] = "CLIENT_REQUEST:",
        client_versus_str[] = "CLIENT_VERSUS\n",
        client_setup_str[19] = "CLIENT_SETUP:",
        client_player_move_str[25] = "CLIENT_PLAYER_MOVE:",
        client_disconnect_str[] = "CLIENT_DISCONNECT\n";

    int i = 0,
        client_request_i = strlen(client_request_str),
        client_setup_i = strlen(client_setup_str),
        client_player_move_i = strlen(client_player_move_str);

    GetStringFromClient(user_input);

    switch (game_state)
    {
    case I_START:
    {
        for (i = 0; i < client_request_i; i++) send_string[i] = client_request_str[i];
        i = 0;
        while (user_input[i] != '\0')
        {
            send_string[i + client_request_i] = user_input[i];
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
            if (user_input[0] = '1') strcpy_s(send_string, strlen(client_versus_str), client_versus_str);
            if (user_input[0] = '2') strcpy_s(send_string, strlen(client_disconnect_str), client_disconnect_str);
            break;
        }
        case SERVER_SETUP_REQUEST:
        {
            i = 0;
            for (i = 0; i < client_setup_i; i++) send_string[i] = client_setup_str[i];
            for (i = 0; i < 4; i++) send_string[i + client_setup_i] = user_input[i];
            send_string[i + client_setup_i] = '\n';
            send_string[i + client_setup_i + 1] = '\0';
            break;
        }
        case SERVER_PLAYER_MOVE_REQUEST:
        {
            i = 0;
            for (i = 0; i < client_player_move_i; i++) send_string[i] = client_player_move_str[i];
            for (i = 0; i < 4; i++) send_string[i + client_player_move_i] = user_input[i];
            send_string[i + client_player_move_i] = '\n';
            send_string[i + client_player_move_i + 1] = '\0';
            break;
        }

        //case SERVER_DENIED_1:
        //{
        //    printf("Server on %s:%s denied the connection request.\nChoose what to do next:\n1. Try to reconnect\n2. Exit\n", server_ip, server_port);
        //    GetStringFromClient(p_game_state, user_input);
        //    DefineStringToSend(p_game_state, user_input, send_string);
        //    SendData(*m_socket, send_string);
        //    RecvData(*m_socket, received_string);
        //    *p_game_state = WhatWasReceived(received_string);
        //    break;
        //}
    }
    return;
}


DWORD WINAPI RecvData(SOCKET* m_socket,char received_string[])
{

    TransferResult_t RecvRes;
    DWORD wait_res;
    BOOL release_res;

    while (1)
    {
        RecvRes = ReceiveString(received_string, *m_socket);

        if (RecvRes == TRNS_FAILED)
        {
            printf("Socket error while trying to read data from socket\n");
            return 0x555;
        }
        else if (RecvRes == TRNS_DISCONNECTED)
        {
            printf("Server closed connection. Bye!\n");
            return 0x555;
        }
        else
        {
            return SUCCESS_CODE;
        }
    }

}


DWORD WINAPI SendData(SOCKET* m_socket, char send_string[])
{

    TransferResult_t SendRes;
    DWORD wait_res;
    BOOL release_res;

    char string_received[5] = NULL;
    char string_to_send[37] = NULL;

    while (1)
    {

        SendRes = SendString(send_string, *m_socket);

        if (SendRes == TRNS_FAILED)
        {
            printf("Socket error while trying to write data to socket\n");
            return 0x555;
        }
    }
}


