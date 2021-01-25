
/*
threadManager.c
----------------------------------------------------------------------------
All thread related actions
*/

//-------------------------------------------------------------//
// --------------------------INCLUDE-------------------------- //
//-------------------------------------------------------------//

#include "threadManager.h"
#include "game.h"

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

void GetClientName(char received_string[], char client_name[]);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Calls a wait for multiple objects on an array with all of the running threads

PARAMETERS - p_threads - an array of thread handles
             number_of_threads - the number of threads to wait for

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int wait_for_threads_execution_and_free(HANDLE ThreadHandles[], SOCKET ThreadInputs[], HANDLE event_two_players, HANDLE TwoPlayerEventThread);

int WhatWasReceived(char* AcceptedStr);

int ReadFromFile(HANDLE file_handle, char string[], int offset);

int StringToFileWithCheck(HANDLE file_handle, char string[], int string_len);

DWORD WINAPI TwoPlayerEventMonitor(LPVOID lpParam);

BOOL PollTwoPlayers(THREAD* thread_params, DWORD time_to_wait, int* p_game_status);

BOOL ThreadCommunicationProtocol(THREAD* thread_params, char string_to_write[], char string_to_read[], int client_0_name_len, int client_1_name_len);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Function every new thread is called to. reads a task from the task file, breaks into primes and prints the correct string to the tasks file. uses a lock regiment as specified

PARAMETERS - lpParam: holds the data structure of pData for that thread

RETURN - signal exit code.
    --------------------------------------------------------------------------------------------*/
DWORD WINAPI ServiceThread(LPVOID lpParam);


HANDLE GetFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition);

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


int wait_for_threads_execution_and_free(HANDLE ThreadHandles[], SOCKET ThreadInputs[], HANDLE event_two_players, HANDLE TwoPlayerEventThread)
{
    int i = 0;
    DWORD dwEvent;

    dwEvent = WaitForMultipleObjects(2, ThreadHandles, FALSE, INFINITE);

    if (WAIT_OBJECT_0 == dwEvent)
    {
        // Free the threads which were dispatched
        for (i = 0; i < NUM_OF_WORKER_THREADS; i++)
        {
            CloseHandle(ThreadHandles[i]);
            closesocket(ThreadInputs[i]);
            printf("freed Thread number: %d\n", i + 1);
        }
    }

    else
    {
        // Print Error message
        WaitError(dwEvent);
        // Free the threads which were dispatched, because some might have been
        for (i = 0; i < 2; i++)
        {
            if (ThreadHandles[i] != NULL)
            {
                CloseHandle(ThreadHandles[i]);
                closesocket(ThreadInputs[i]);
                printf("freed Thread number: %d\n", i + 1);
            }
        }
    }
	CloseHandle(event_two_players);
	CloseHandle(TwoPlayerEventThread);
    //if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}


int WhatWasReceived(char* AcceptedStr)
{
    if (0 == strncmp(AcceptedStr, "CLIENT_REQUEST", strlen("CLIENT_REQUEST") - 2)) return CLIENT_REQUEST;
    if (0 == strncmp(AcceptedStr, "CLIENT_VERSUS", strlen("CLIENT_VERSUS") - 2)) return CLIENT_VERSUS;
    if (0 == strncmp(AcceptedStr, "CLIENT_SETUP", strlen("CLIENT_SETUP") - 2)) return CLIENT_SETUP;
    if (0 == strncmp(AcceptedStr, "CLIENT_PLAYER_MOVE", strlen("CLIENT_PLAYER_MOVE") - 2)) return CLIENT_PLAYER_MOVE;
    if (0 == strncmp(AcceptedStr, "CLIENT_DISCONNECT", strlen("CLIENT_DISCONNECT") - 2)) return CLIENT_DISCONNECT;

    // Upon failure
    return STATUS_CODE_FAILURE;
}


int ReadFromFile(HANDLE file_handle, char string[], int offset)
{
	char curr_char = '\0';
	//HANDLE file_handle = GetFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);			//check for failures
	int i = 0;
	// offset string
	DWORD dwPtr = SetFilePointer(file_handle, offset, NULL, FILE_BEGIN);

	// read string
	while (curr_char != '\r')
	{
		if (FALSE == ReadFile(file_handle, &curr_char, 1, NULL, NULL))
		{
			printf("couldn't open file to be read in WINAPI\n");
			return STATUS_CODE_FAILURE;
		}

		if (curr_char == '\r') string[i] = '\0';
		else string[i] = curr_char;
		i++;
	}

	//Success
	//CloseHandle(file_handle);
	return SUCCESS_CODE;
}


void GetClientName(char received_string[], char client_name[])
{
	int i;

	for (i = 0; i < strlen(received_string) - strlen("CLIENT_REQUEST:") - 1 ; i++) client_name[i] = received_string[strlen("CLIENT_REQUEST:") + i];

	client_name[i] = '\r';
	client_name[i + 1] = '\n';
	client_name[i + 2] = '\0';
}


int StringToFileWithCheck(HANDLE file_handle, char string[], int string_len)
{
	DWORD dwPtr = SetFilePointer(file_handle, 0, NULL, FILE_END);

	if (FALSE == WriteFile(file_handle, string, string_len, NULL, NULL))
	{
		printf("couldn't open file to be write in WINAPI\n");
		CloseHandle(file_handle);
		return STATUS_CODE_FAILURE;
	}
	return SUCCESS_CODE;
}


DWORD WINAPI TwoPlayerEventMonitor(LPVOID lpParam)
{
	TWO_PLAYER_THREAD* thread_params = (TWO_PLAYER_THREAD*)lpParam;
	while (1)
	{
		if (*thread_params->p_number_of_clients_connected == 2) SetEvent(*thread_params->p_event);
		else if (*thread_params->p_number_of_clients_connected < 2) ResetEvent(*thread_params->p_event);
		else printf("more than 2 clients have connected");
	}
}


BOOL PollTwoPlayers(THREAD* thread_params, DWORD time_to_wait, int* p_game_status)
{
	DWORD wait_res;
	if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_event, time_to_wait)))
	{
		(*thread_params->p_number_of_clients_connected)--;
		if (time_to_wait == WAIT_FOR_OTHER_PLAYER_IN_MILLISECONDS) *p_game_status = CLIENT_REQUEST;
		else if (time_to_wait == POLL_EVENT_STATUS) *p_game_status = OPPONENT_QUIT;
		return FALSE;
	}
	// Only thread[0] opens the file
	if (thread_params->thread_id == 0)
	{
		*thread_params->file_handle = GetFile(thread_params->tasks_file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, CREATE_ALWAYS);
	}
	return TRUE;
}


BOOL ThreadCommunicationProtocol(THREAD* thread_params, char string_to_write[], char string_to_read[], int client_0_name_len, int client_1_name_len)
{
	DWORD wait_res;
	BOOL release_res;

	if (thread_params->thread_id == 0)
	{
		if (STATUS_CODE_FAILURE == StringToFileWithCheck(*thread_params->file_handle, string_to_write, strlen(string_to_write) + 1)) return STATUS_CODE_FAILURE;
		if (FALSE == (release_res = ReleaseMutex(*thread_params->p_mutex_file)))
		{
			printf("Couldn't release Mutex");
			return release_res;
		}
		Sleep(1000);
		if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME)))
		{
			printf("waited too long for mutex");
			return FALSE;
		}
		ReadFromFile(*thread_params->file_handle, string_to_read, (strlen(string_to_write) + 1));
		(*thread_params->stage_of_game)++;
	}
	else
	{
		if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME)))
		{
			printf("waited too long for mutex");
			return FALSE;
		}
		ReadFromFile(*thread_params->file_handle, string_to_read, 0);
		if (STATUS_CODE_FAILURE == StringToFileWithCheck(*thread_params->file_handle, string_to_write, strlen(string_to_write) + 1)) return STATUS_CODE_FAILURE;
		if (FALSE == (release_res = ReleaseMutex(*thread_params->p_mutex_file)))
		{
			printf("Couldn't release Mutex");
			return release_res;
		}
	}
}

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
DWORD WINAPI ServiceThread(LPVOID lpParam)
{
	char send_string[MAX_BYTES_SERVER_MIGHT_SEND];
	char received_string[MAX_BYTES_SERVER_MIGHT_SEND];
	char client_number[7] = "0000\r\n";
	char opponent_number[7] = "0000\r\n";
	char client_guess[7] = "0000\r\n";
	char opponent_guess[7] = "0000\r\n";
	char client_name[23] = "00000000000000000000\r\n";
	char opponent_name[23] = "00000000000000000000\r\n";

	char server_main_menu_str[] = "SERVER_MAIN_MENU\n",
		server_approved_str[] = "SERVER_APPROVED\n",
		server_denied_1_str[] = "SERVER_DENIED\n",
		server_denied_2_str[] = "SERVER_DENIED\n",
		server_invite[] = "SERVER_INVITE:",
		server_invite_str[36],
		server_setup_request_str[] = "SERVER_SETUP_REQUEST\n",
		server_player_move_request_str[] = "SERVER_PLAYER_MOVE_REQUEST\n",
		server_game_results_str[66],
		server_win_str[38],
		server_draw_str[] = "SERVER_DRAW\n",
		server_no_opponents_str[] = "SERVER_NO_OPPONENTS\n",
		server_opponent_quit_str[] = "SERVER_OPPONENT_QUIT\n";

	THREAD* thread_params = (THREAD*)lpParam;
	printf("My thread ID is %d\n", thread_params->thread_id);
	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	int i = 0, game_state = I_START, game_result = GAME_CONTINUES;
	int* p_game_result = &game_result;

	SOCKET* ThreadInputs;

	DWORD wait_res;
	BOOL release_res;

	while (game_state != CLIENT_DISCONNECT)
	{
		switch (game_state)
		{
			case I_START:
			{
				// Only thread[0] opens the file
				if (thread_params->thread_id == 0)
				{
					if (NULL == (*thread_params->p_mutex_file = CreateMutex(NULL, TRUE, NULL))) CloseHandle(thread_params->p_mutex_file);  /// add a return
				}
				RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string);
				printf("%s", received_string);
				//Server approved
				//strcpy_s(send_string, strlen(server_approved_str), server_approved_str);
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_approved_str);
				GetClientName(received_string, client_name);
				game_state = WhatWasReceived(received_string);
				break;
			}
			case CLIENT_REQUEST:
			{
				//Server Main Menu
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_main_menu_str);
				//wait forever																								//Something special here?
				RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string);
				printf("%s", received_string);
				game_state = WhatWasReceived(received_string);
				break;
			}
			case CLIENT_VERSUS:
			{
				printf("My thread ID is %d\n", thread_params->thread_id);
				//signal one half of the event
				//wait for the event to be fully signaled for 15 seconds
				(*thread_params->p_number_of_clients_connected)++;
				if (FALSE == PollTwoPlayers(thread_params, WAIT_FOR_OTHER_PLAYER_IN_MILLISECONDS, &game_state))
				{
					SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_no_opponents_str);
				}
				// Two opponents are connected
				else
				{
					*thread_params->stage_of_game = 0;
					ThreadCommunicationProtocol(thread_params, client_name, opponent_name, strlen(client_name) + 1, strlen(opponent_name) + 1);			//Find a better ending

					// Save opponents name
					for (i = 0; i < strlen(server_invite); i++)
					{
						server_invite_str[i] = server_invite[i];
					}
					i = 0;
					while (opponent_name[i] != '\0')
					{
						server_invite_str[i + strlen(server_invite)] = opponent_name[i];
						i++;
					}
					server_invite_str[i + strlen(server_invite)] = '\n';
					server_invite_str[i + 1 + strlen(server_invite)] = '\0';

					// Server invite
					if (FALSE == PollTwoPlayers(thread_params, POLL_EVENT_STATUS, &game_state)) break;
					SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_invite_str);
					// Server Setup request
					if (FALSE == PollTwoPlayers(thread_params, POLL_EVENT_STATUS, &game_state)) break;
					SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_setup_request_str);
					RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string);
					printf("%s", received_string);
					// Copy the clients number including the NULL sign
					for (i = 0; i < 5; i++) client_number[i] = received_string[i + 13];
					client_number[4] = '\r';
					client_number[5] = '\n';
					client_number[6] = '\0';

					ThreadCommunicationProtocol(thread_params, client_number, opponent_number, strlen(client_name) + 1, strlen(opponent_name) + 1);				//Find a better ending

					game_state = WhatWasReceived(received_string);
				}
				break;
			}
			case CLIENT_SETUP:
			{
				if (FALSE == PollTwoPlayers(thread_params, POLL_EVENT_STATUS, &game_state)) break;
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_player_move_request_str);
				RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string);
				printf("%s", received_string);
				for (i = 0; i < 5; i++) client_guess[i] = received_string[i + 19];
				client_guess[4] = '\r';
				client_guess[5] = '\n';
				client_guess[6] = '\0';
				ThreadCommunicationProtocol(thread_params, client_guess, opponent_guess, strlen(client_name), strlen(opponent_name));				//Find a better ending

				game_state = WhatWasReceived(received_string);
				break;
			}
			case CLIENT_PLAYER_MOVE:
			{
				if (FALSE == PollTwoPlayers(thread_params, POLL_EVENT_STATUS, &game_state)) break;
				// Get server_game_results_str string or server_win_str string
				GetGameResults(client_number, opponent_number, client_guess, opponent_guess, client_name, opponent_name, server_game_results_str, server_win_str, p_game_result);

				if (game_result == GAME_WON)
				{
					(*thread_params->p_number_of_clients_connected)--;
					SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_win_str);
					game_state = CLIENT_REQUEST;
				
				}
				else if (game_result == GAME_DRAW)
				{
					(*thread_params->p_number_of_clients_connected)--;
					SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_draw_str);
					game_state = CLIENT_REQUEST;
				}
				else
				{
					SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_game_results_str);
					game_state = CLIENT_SETUP;
				}
				break;
			}
			case OPPONENT_QUIT:
			{
				(*thread_params->p_number_of_clients_connected)--;
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_opponent_quit_str);
				game_state = CLIENT_REQUEST;
				break;
			}
		}
		if (game_state == CLIENT_DISCONNECT)
		{
			(*thread_params->p_number_of_clients_connected)--;
			// Only thread[0] opens the file
			if (thread_params->thread_id == 0)
			{
				//close mutex
				//close file
			}
		}
		continue;
	}
	return 0;
}


HANDLE GetFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition) {
	HANDLE file_handle;

	if (NULL == lpFileName || 0 == dwDesiredAccess || 0 == dwShareMode)
	{
		printf("Received null pointer");
		return NULL;
	}

	file_handle = CreateFileA(
		lpFileName,				//The name of the file or device to be created or opened.
		dwDesiredAccess,        //The requested access to the file or device, which can be summarized 
								//as read, write, both or neither zero).
								//The most commonly used values are 
								//GENERIC_READ, GENERIC_WRITE, or both(GENERIC_READ | GENERIC_WRITE).
		dwShareMode,            //The requested sharing mode of the file or device, which can be read, 
								//write, both, delete, all of these, or none 
								//FILE_SHARE_READ 1 OR FILE_SHARE_WRITE 2
		NULL,
		dwCreationDisposition,  //Should be CREATE_ALWAYS to overwrite or OPEN_EXISTING TO READ
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (NULL == file_handle)
	{
		printf("Couldn't create file\n");
		return NULL;
	}

	return file_handle;
}
