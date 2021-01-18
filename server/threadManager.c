
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
int wait_for_threads_execution_and_free(HANDLE ThreadHandles[], SOCKET ThreadInputs[]);


int WhatWasReceived(char* AcceptedStr);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Function every new thread is called to. reads a task from the task file, breaks into primes and prints the correct string to the tasks file. uses a lock regiment as specified

PARAMETERS - lpParam: holds the data structure of pData for that thread

RETURN - signal exit code.
    --------------------------------------------------------------------------------------------*/
DWORD WINAPI ServiceThread(LPVOID lpParam);


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


int wait_for_threads_execution_and_free(HANDLE ThreadHandles[], SOCKET ThreadInputs[])
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
        WaitError(dwEvent, 0);
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

    if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}


int WhatWasReceived(char* AcceptedStr)
{
    if (0 == strcmp(AcceptedStr, "CLIENT_REQUEST")) return CLIENT_REQUEST;
    if (0 == strcmp(AcceptedStr, "CLIENT_VERSUS")) return CLIENT_VERSUS;
    if (0 == strcmp(AcceptedStr, "CLIENT_SETUP")) return CLIENT_SETUP;
    if (0 == strcmp(AcceptedStr, "CLIENT_PLAYER_MOVE")) return CLIENT_PLAYER_MOVE;
    if (0 == strcmp(AcceptedStr, "CLIENT_DISCONNECT")) return CLIENT_DISCONNECT;

    // Upon failure
    return STATUS_CODE_FAILURE;
}


ReadFromFile(char filename[], char string[], int string_len, int offset)
{
	HANDLE file_handle = GetFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);
	int i = 0;
	// offset string
	DWORD dwPtr = SetFilePointer(file_handle, offset, NULL, FILE_BEGIN);

	// read string
	for (i = 0; i < string_len; i++)
	{
		if (FALSE == ReadFile(file_handle, string[i], 1, NULL, NULL)); ///return error
		if (string[i] = '\r') break;
	}
	string[i] = '\0';

	//Success
	CloseHandle(file_handle);
	return SUCCESS_CODE;
}


int PrintToFile(char filename[], char string[], int string_len)
{
	HANDLE file_handle = GetFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);

	// write string
	if (STATUS_CODE_FAILURE == StringToFileWithCheck(file_handle, string, string_len)) return STATUS_CODE_FAILURE;

	//Success
	CloseHandle(file_handle);
	return SUCCESS_CODE;
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


//Service thread is the thread that opens for each successful client connection and "talks" to the client.
DWORD WINAPI ServiceThread(LPVOID lpParam)
{
	char send_string[MAX_BYTES_SERVER_MIGHT_SEND];
	char received_string[MAX_BYTES_CLIENT_MIGHT_SEND];
	char client_number[5] = "0000";
	char opponent_number[5] = "0000";
	char client_guess[5] = "0000";
	char opponent_guess[5] = "0000";
	char client_name[21] = "00000000000000000000";
	char opponent_name[21] = "00000000000000000000";

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
	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	int i = 0, game_state = I_START;

	HANDLE file_handle;
	SOCKET* ThreadInputs;
	LOCK* p_lock;
	char* tasks_file_name;
	int thread_id;

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
				if (NULL == (*thread_params->p_mutex_file = CreateMutex(NULL, TRUE, NULL))) CloseHandle(*thread_params->p_mutex_file);  /// add a return
			}
			RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string);
			//Server approved
			strcpy_s(send_string, strlen(server_approved_str), server_approved_str);
			SendData((thread_params->ThreadInputs)[thread_params->thread_id], send_string);
			GetClientName(received_string, client_name);
			game_state = WhatWasReceived(received_string);
			break;
		}
		case CLIENT_REQUEST:
		{
			//Server Main Menu
			strcpy_s(send_string, strlen(server_main_menu_str), server_main_menu_str);
			SendData((thread_params->ThreadInputs)[thread_params->thread_id], send_string);
			//wait forever
			RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string);
			game_state = WhatWasReceived(received_string);
			break;
		}
		case CLIENT_VERSUS:
		{
			//signal one half of the event
			//wait for the event to be fully signaled for 15 seconds
			if (timeout_wait)
			{
				strcpy_s(send_string, strlen(server_no_opponents_str), server_no_opponents_str);
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], send_string);
				game_state = CLIENT_REQUEST;
			}
			// Two opponents are connected
			else
			{
				// Only thread[0] opens the file
				if (thread_params->thread_id == 0) 	file_handle = GetFile(thread_params->tasks_file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, CREATE_ALWAYS);
				*thread_params->stage_of_game = 0;
				if (thread_params->thread_id == 0)
				{
					PrintToFile(thread_params->tasks_file_name, client_name, strlen(client_name));
					if (FALSE == (release_res = ReleaseMutex(*thread_params->p_mutex_file)))
					{
						printf("Couldn't release Mutex");
						return wait_res;
					}
					if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME))) return wait_res;
					ReadFromFile(thread_params->tasks_file_name, opponent_name, strlen(opponent_name), thread_params->stage_of_game);				//need to calculate offset better

				}
				else
				{
					if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME))) return wait_res;
					ReadFromFile(thread_params->tasks_file_name, opponent_name, strlen(opponent_name), thread_params->stage_of_game);				//need to calculate offset better
					PrintToFile(thread_params->tasks_file_name, client_name, strlen(client_name));
					if (FALSE == (release_res = ReleaseMutex(*thread_params->p_mutex_file)))
					{
						printf("Couldn't release Mutex");
						return wait_res;
					}
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
				server_invite_str[i + strlen(server_invite)] = '\0';

				*thread_params->stage_of_game++;

				// Server invite
				strcpy_s(send_string, strlen(server_invite_str), server_invite_str);
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], send_string);
				// Server Setup request
				strcpy_s(send_string, strlen(server_setup_request_str), server_setup_request_str);
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], send_string);
				RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string);
				// Copy the clients number including the NULL sign
				for (i = 0; i < 5; i++) client_number[i] = received_string[i + 13];

				if (thread_params->thread_id == 0)
				{
					PrintToFile(thread_params->tasks_file_name, client_number, strlen(client_number));
					if (FALSE == (release_res = ReleaseMutex(*thread_params->p_mutex_file)))
					{
						printf("Couldn't release Mutex");
						return wait_res;
					}					if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME))) return wait_res;
					ReadFromFile(thread_params->tasks_file_name, opponent_number, strlen(opponent_number), thread_params->stage_of_game);				//need to calculate offset better
				}
				else
				{
					if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME))) return wait_res;
					ReadFromFile(thread_params->tasks_file_name, opponent_number, strlen(opponent_number), thread_params->stage_of_game);				//need to calculate offset better
					PrintToFile(thread_params->tasks_file_name, client_number, strlen(client_number));
					if (FALSE == (release_res = ReleaseMutex(*thread_params->p_mutex_file)))
					{
						printf("Couldn't release Mutex");
						return wait_res;
					}
				}
				*thread_params->stage_of_game++;
				game_state = WhatWasReceived(received_string);
			}
			break;
		}
		case CLIENT_SETUP:
		{
			strcpy_s(send_string, strlen(server_player_move_request_str), server_player_move_request_str);
			SendData((thread_params->ThreadInputs)[thread_params->thread_id], send_string);
			RecvData((thread_params->ThreadInputs)[thread_params->thread_id], received_string);
			for (i = 0; i < 5; i++) client_guess[i] = received_string[i + 19];

			if (thread_params->thread_id == 0)
			{
				PrintToFile(thread_params->tasks_file_name, client_guess, strlen(client_guess));
				if (FALSE == (release_res = ReleaseMutex(*thread_params->p_mutex_file)))
				{
					printf("Couldn't release Mutex");
					return wait_res;
				}				if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME))) return wait_res;
				ReadFromFile(thread_params->tasks_file_name, opponent_guess, strlen(opponent_guess), thread_params->stage_of_game);				//need to calculate offset better
			}
			else
			{
				if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*thread_params->p_mutex_file, THREAD_WAIT_TIME))) return wait_res;
				ReadFromFile(thread_params->tasks_file_name, opponent_guess, strlen(opponent_guess), thread_params->stage_of_game);				//need to calculate offset better
				PrintToFile(thread_params->tasks_file_name, client_guess, strlen(client_guess));
				if (FALSE == (release_res = ReleaseMutex(*thread_params->p_mutex_file)))
				{
					printf("Couldn't release Mutex");
					return wait_res;
				}
			}
			*thread_params->stage_of_game++;
			game_state = WhatWasReceived(received_string);
			break;
		}
		case CLIENT_PLAYER_MOVE:
		{
			// Get server_game_results_str string or server_win_str string
			if (a player won)
			{
				strcpy_s(send_string, strlen(server_win_str), server_win_str);
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], send_string);
				game_state = CLIENT_REQUEST;
				if (thread_params->thread_id == 0)
				{
					//clean the file
				}
				
			}
			else if (it is a tie)
			{
				strcpy_s(send_string, strlen(server_draw_str), server_draw_str);
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], send_string);
				game_state = CLIENT_REQUEST;
				if (thread_params->thread_id == 0)
				{
					//clean the file
				}
			}
			else
			{
				strcpy_s(send_string, strlen(server_game_results_str), server_game_results_str);
				SendData((thread_params->ThreadInputs)[thread_params->thread_id], send_string);
				game_state = CLIENT_SETUP;
			}
			break;
		}
		}
		if (game_state = CLIENT_DISCONNECT)
		{
			// Only thread[0] opens the file
			if (thread_params->thread_id == 0)
			{
				//close mutex
			}
		}
		continue;
	}
	return 0;
}