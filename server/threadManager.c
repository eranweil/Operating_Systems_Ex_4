
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
DESCRIPTION - saves the clients name from the recieved string

PARAMETERS - received_string, client_name

RETURN - void
	--------------------------------------------------------------------------------------------*/
void GetClientName(char received_string[], char client_name[]);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - free all threads, sockets and handles

PARAMETERS - all threads, sockets and handles

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int FreeAll(HANDLE ThreadHandles[], SOCKET ThreadInputs[], HANDLE* event_two_players, HANDLE* TwoPlayerEventThread, HANDLE* ExitThread, HANDLE* ExitEvent, HANDLE* file_handle, HANDLE* mutex_file, HANDLE* event_player_2, SOCKET* MainSocket, char game_file_name[]);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - check what the client sent

PARAMETERS - client sent string

RETURN - success code upon success or failure code otherwise
	--------------------------------------------------------------------------------------------*/
int WhatWasReceived(char* AcceptedStr);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - reads from the contact file between the threads

PARAMETERS - file_handle, string buffer to read to, offset to read from

RETURN - success code upon success or failure code otherwise
	--------------------------------------------------------------------------------------------*/
int ReadFromFile(HANDLE file_handle, char string[], int offset);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - writes to the contact file between the threads

PARAMETERS - file_handle, string buffer to write, string len

RETURN - success code upon success or failure code otherwise
	--------------------------------------------------------------------------------------------*/
int StringToFileWithCheck(HANDLE file_handle, char string[], int string_len);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - the function for the thread that checks if there are two players or not
	--------------------------------------------------------------------------------------------*/
DWORD WINAPI TwoPlayerEventMonitor(LPVOID lpParam);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - the function for the thread that checks if there was an exit
	--------------------------------------------------------------------------------------------*/
DWORD WINAPI ExitMonitor(LPVOID lpParam);

/*--------------------------------------------------------------------------------------------
The threads function to check if there are two players (based on the result of TwoPlayerEventMonitor)
	--------------------------------------------------------------------------------------------*/
BOOL PollTwoPlayers(THREAD* thread_params, DWORD time_to_wait, int* p_game_status);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - the protocol to read and write to the joint file while syncing

PARAMETERS - string_to_write buffer, string_to_read buffer, length of the strings and the thread params which have pointers to all of the locks

RETURN - signal exit code.
	--------------------------------------------------------------------------------------------*/
BOOL ThreadCommunicationProtocol(THREAD* thread_params, char string_to_write[], char string_to_read[], int client_0_name_len, int client_1_name_len);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - The main thread to work with incoming clients
    --------------------------------------------------------------------------------------------*/
DWORD WINAPI ServiceThread(LPVOID lpParam);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Simplified function to open a file
	--------------------------------------------------------------------------------------------*/
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


int FreeAll(HANDLE ThreadHandles[], SOCKET ThreadInputs[], HANDLE* event_two_players, HANDLE* TwoPlayerEventThread, HANDLE* ExitThread, HANDLE* ExitEvent, HANDLE* file_handle, HANDLE* mutex_file, HANDLE* event_player_2, SOCKET* MainSocket, char game_file_name[])
{
    int i = 0;
    DWORD dwEvent;


	dwEvent = WaitForMultipleObjects(2, ThreadHandles, FALSE, THREAD_WAIT_TIME);

    if (WAIT_OBJECT_0 == dwEvent)
    {
        // Free the threads which were dispatched
		for (i = 0; i < NUM_OF_WORKER_THREADS; i++)
		{
			if (ThreadHandles[i] != NULL)
			{
				CloseHandle(ThreadHandles[i]);
				closesocket(ThreadInputs[i]);
				printf("freed Thread number: %d\n", i + 1);
			}
		}
    }
	if (FALSE == (CloseHandle(*event_two_players))) return STATUS_CODE_FAILURE;
	if (FALSE == (CloseHandle(*TwoPlayerEventThread))) return STATUS_CODE_FAILURE;
	if (FALSE == (CloseHandle(*ExitEvent))) return STATUS_CODE_FAILURE;
	if (FALSE == (CloseHandle(*ExitThread))) return STATUS_CODE_FAILURE;
	if (FALSE == (CloseHandle(*event_player_2))) return STATUS_CODE_FAILURE;
	if (NULL != *file_handle)
	{
		if (FALSE == (CloseHandle(*file_handle))) return STATUS_CODE_FAILURE;
		//if (FALSE == (DeleteFileA(game_file_name))) return STATUS_CODE_FAILURE;
	}	
	if (NULL != *mutex_file)
	{
		if (FALSE == (CloseHandle(*mutex_file))) return STATUS_CODE_FAILURE;
	}

	if (closesocket(*MainSocket) == SOCKET_ERROR)
	{
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return STATUS_CODE_FAILURE;
	}

	return SUCCESS_CODE;
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
	return SUCCESS_CODE;
}


void GetClientName(char received_string[], char client_name[])
{
	unsigned int i;

	for (i = 0; i < (strlen(received_string) - strlen("CLIENT_REQUEST:") - 1) ; i++) client_name[i] = received_string[strlen("CLIENT_REQUEST:") + i];

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
	while (WAIT_OBJECT_0 != WaitForSingleObject(*thread_params->p_ExitEvent, 0))
	{
		if (*thread_params->p_number_of_clients_connected == 2) SetEvent(*thread_params->p_event);
		else if (*thread_params->p_number_of_clients_connected < 2) ResetEvent(*thread_params->p_event);
		else printf("more than 2 clients have connected");
	}
	return SUCCESS_CODE;
}


DWORD WINAPI ExitMonitor(LPVOID lpParam)
{
	HANDLE* p_ExitEvent = (HANDLE*)lpParam;
	ResetEvent(*p_ExitEvent);
	char exit_word[5] = "exit";
	char input_word[5] = "0000";
	while (0 != strcmp(exit_word, input_word))
	{
		scanf_s("%s", input_word, 5);
	}
	SetEvent(*p_ExitEvent);
	return SUCCESS_CODE;
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
		if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->event_player_2, INFINITE)))
		{
			printf("waited too long for player 2 to write to file");
			return FALSE;
		}
		if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME)))
		{
			printf("waited too long for file mutex");
			return FALSE;
		}
		if (STATUS_CODE_FAILURE == ReadFromFile(*thread_params->file_handle, string_to_read, (strlen(string_to_write) + 1))) return STATUS_CODE_FAILURE;
	}
	else
	{
		if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME)))
		{
			printf("waited too long for mutex");
			return FALSE;
		}
		if (STATUS_CODE_FAILURE == ReadFromFile(*thread_params->file_handle, string_to_read, 0)) return STATUS_CODE_FAILURE;
		if (STATUS_CODE_FAILURE == StringToFileWithCheck(*thread_params->file_handle, string_to_write, strlen(string_to_write) + 1)) return STATUS_CODE_FAILURE;
		SetEvent(*thread_params->event_player_2);
		if (FALSE == (release_res = ReleaseMutex(*thread_params->p_mutex_file)))
		{
			printf("Couldn't release Mutex");
			return release_res;
		}
	}
	return release_res;
}

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
DWORD WINAPI ServiceThread(LPVOID lpParam)
{
	char received_string[MAX_BYTES_SERVER_MIGHT_SEND];
	char client_number[7] = "0000\r\n";
	char opponent_number[7] = "0000\r\n";
	char client_guess[7] = "0000\r\n";
	char opponent_guess[7] = "0000\r\n";
	char client_name[23] = "00000000000000000000\r\n";
	char opponent_name[23] = "00000000000000000000\r\n";

	char server_main_menu_str[] = "SERVER_MAIN_MENU\n",
		server_approved_str[] = "SERVER_APPROVED\n",
		server_invite[] = "SERVER_INVITE:",
		server_invite_str[36],
		server_setup_request_str[] = "SERVER_SETUP_REQUEST\n",
		server_player_move_request_str[] = "SERVER_PLAYER_MOVE_REQUEST\n",
		server_game_results_str[66],
		server_win[38],
		server_win_str[38],
		server_draw_str[] = "SERVER_DRAW\n",
		server_no_opponents_str[] = "SERVER_NO_OPPONENTS\n",
		server_opponent_quit_str[] = "SERVER_OPPONENT_QUIT\n",
		server_denied_str[] = "SERVER_DENIED\n";


	THREAD* thread_params = (THREAD*)lpParam;
	printf("My thread ID is %d\n", thread_params->thread_id);


	unsigned int i = 0;
	BOOL first_game = TRUE;
	int game_state = I_START, game_result = GAME_CONTINUES;
	int* p_game_result = &game_result;
	int len_client = 0, len_opponent = 0;

	while (game_state != CLIENT_DISCONNECT)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(*thread_params->p_ExitEvent, 0))
		{
			if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_denied_str))) return STATUS_CODE_FAILURE;
			(*thread_params->p_number_of_clients_connected)--;
			return SUCCESS_CODE;
		}
		switch (game_state)
		{
			case I_START:
			{
				// Only thread[0] opens the file
				if (thread_params->thread_id == 0)
				{
					if (NULL == (*thread_params->p_mutex_file = CreateMutex(NULL, TRUE, NULL))) CloseHandle(thread_params->p_mutex_file);  /// add a return
				}
				//Server approved
				if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_approved_str))) return STATUS_CODE_FAILURE;
				if (STATUS_CODE_FAILURE == (RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string))) return STATUS_CODE_FAILURE;
				printf("\n");
				GetClientName(received_string, client_name);
				game_state = WhatWasReceived(received_string);
				break;
			}
			case CLIENT_REQUEST:
			{
				//Server Main Menu
				if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_main_menu_str))) return STATUS_CODE_FAILURE;
				//wait forever
				if (STATUS_CODE_FAILURE == (RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string))) return STATUS_CODE_FAILURE;
				printf("\n");
				game_state = WhatWasReceived(received_string);
				break;
			}			
			case SPECIAL_CLIENT_REQUEST:
			{
				Sleep(2000);
				//Server Main Menu
				if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_main_menu_str))) return STATUS_CODE_FAILURE;
				//wait forever
				(*thread_params->p_number_of_clients_connected)--;
				if (STATUS_CODE_FAILURE == (RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string))) return STATUS_CODE_FAILURE;
				printf("\n");
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
					if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_no_opponents_str))) return STATUS_CODE_FAILURE;
				}
				// Two opponents are connected
				else
				{
					if (first_game)
					{
						first_game = FALSE;
						ThreadCommunicationProtocol(thread_params, client_name, opponent_name, strlen(client_name) + 1, strlen(opponent_name) + 1);			//Find a better ending
					}
					else
					{
						len_client = strlen(client_name);
						len_opponent = strlen(opponent_name);
						client_name[len_client] = '\r';
						client_name[len_client + 1] = '\n';
						client_name[len_client + 2] = '\0';
						opponent_name[len_opponent] = '\r';
						opponent_name[len_opponent + 1] = '\n';
						opponent_name[len_opponent + 2] = '\0';
						ThreadCommunicationProtocol(thread_params, client_name, opponent_name, strlen(client_name) + 1, strlen(opponent_name) + 1);			//Find a better ending
					}

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
					if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_invite_str))) return STATUS_CODE_FAILURE;
					// Server Setup request
					if (FALSE == PollTwoPlayers(thread_params, POLL_EVENT_STATUS, &game_state)) break;
					if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_setup_request_str))) return STATUS_CODE_FAILURE;
					if (STATUS_CODE_FAILURE == (RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string))) return STATUS_CODE_FAILURE;
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
				if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_player_move_request_str))) return STATUS_CODE_FAILURE;
				if (STATUS_CODE_FAILURE == (RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string))) return STATUS_CODE_FAILURE;
				printf("\n");
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
				GetGameResults(client_number, opponent_number, client_guess, opponent_guess, client_name, opponent_name, server_game_results_str, server_win, p_game_result);

				if (game_result == GAME_WON)
				{
					for (i = 0; i < strlen(server_win) - 1; i++) server_win_str[i] = server_win[i];
					server_win_str[i] = ';';
					for (i = 0; i < 4; i++) server_win_str[i + strlen(server_win)] = opponent_number[i];
					server_win_str[i + strlen(server_win)] = '\n';
					server_win_str[i + strlen(server_win) + 1] = '\0';

					if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_win_str))) return STATUS_CODE_FAILURE;
					game_state = SPECIAL_CLIENT_REQUEST;
				}
				else if (game_result == GAME_DRAW)
				{
					if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_draw_str))) return STATUS_CODE_FAILURE;
					game_state = SPECIAL_CLIENT_REQUEST;
				}
				else
				{
					if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_game_results_str))) return STATUS_CODE_FAILURE;
					game_state = CLIENT_SETUP;
				}
				break;
			}
			case OPPONENT_QUIT:
			{
				if (STATUS_CODE_FAILURE == (SendData((thread_params->ThreadInputs)[thread_params->thread_id], server_opponent_quit_str))) return STATUS_CODE_FAILURE;
				game_state = SPECIAL_CLIENT_REQUEST;
				break;
			}
		}
	}
	return SUCCESS_CODE;
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
