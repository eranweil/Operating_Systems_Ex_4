

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

#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
#include "SocketSendRecvTools.h"
#include "game.h"


int check_arguments(int argc, char** argv)
{
	// making sure there are enough args given
	if (argc - 1 < ARGS_REQUIRED_SERVER)
	{
		printf("Too few arguments given. We need %d arguments and you have provided %d\n", ARGS_REQUIRED_SERVER, argc - 1);
		return STATUS_CODE_FAILURE;
	}

	if (argc - 1 > ARGS_REQUIRED_SERVER)
	{
		printf("Too many arguments given. We need %d arguments and you have provided %d\n", ARGS_REQUIRED_SERVER, argc - 1);
		return STATUS_CODE_FAILURE;
	}

	if (argc - 1 == ARGS_REQUIRED_SERVER)
	{
		printf("The port number is %s\n", *(argv + 1));
	}
	return SUCCESS_CODE;
}


int main(int argc, char** argv)
{
	char server_ip[] = "127.0.0.1";
	char server_port[] = "8888";
	char game_file_name[]= "GameSession.txt";
	char event_name[] = "TWO_PLAYER_EVENT";
	int i = 0;
	int j = 0;
	for (i = 0; i < 4; i++) server_port[i] = *((*(argv + 1)) + i);

	int number_of_clients_connected = 0;
	int* p_number_of_clients_connected = &number_of_clients_connected;
	int stage_of_game = 0;
	int server_port_num = atoi(server_port);
	int* p_stage_of_game = &stage_of_game;

	HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	HANDLE TwoPlayerEventThread;
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
	HANDLE file_handle;

	HANDLE mutex_file;
	HANDLE event_two_players;

	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	THREAD Data_0;
	THREAD Data_1;
	THREAD* pData_0 = &Data_0;
	THREAD* pData_1 = &Data_1;
	TWO_PLAYER_THREAD two_player_data;
	TWO_PLAYER_THREAD* p_two_player_data = &two_player_data;
	DWORD dwThreadId[NUM_OF_WORKER_THREADS];
	DWORD dwTwoPlayerEventThreadId;

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
	/* The WinSock DLL is acceptable. Proceed. */

	// Create a socket.    
	if ((MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return;
	}

	// Bind the socket.
	if ((Address = inet_addr(server_ip)) == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n", server_ip);
		if (closesocket(MainSocket) == SOCKET_ERROR)
		{
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
			return;
		}
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(server_port_num); 

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	if ((bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service))) == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		if (closesocket(MainSocket) == SOCKET_ERROR)
		{
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
			return;
		}
	}

	// Listen on the Socket.
	if ((ListenRes = listen(MainSocket, SOMAXCONN)) == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		if (closesocket(MainSocket) == SOCKET_ERROR)
		{
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
			return;
		}
	}

	// Create the thread that listens to two players connected
	event_two_players = CreateEvent(
		NULL,					/* default security attributes */
		TRUE,					/* manual-reset event */
		FALSE,					/* initial state is non-signaled */
		event_name);			/* name */

	p_two_player_data->p_number_of_clients_connected = p_number_of_clients_connected;
	p_two_player_data->p_event = &event_two_players;

	TwoPlayerEventThread = CreateThread(
		NULL,
		0,
		TwoPlayerEventMonitor,
		p_two_player_data,
		0,
		&dwTwoPlayerEventThreadId
	);

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (i = 0; i < NUM_OF_WORKER_THREADS; i++) ThreadHandles[i] = NULL;

	printf("Waiting for a client to connect...\n");

	for (j = 0; j < MAX_LOOPS; j++)																//(while no quit)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			for (i = 0; i < NUM_OF_WORKER_THREADS; i++)
			{
				if (ThreadHandles[i] != NULL)
				{
					// poll to check if thread finished running:
					DWORD Res = WaitForSingleObject(ThreadHandles[i], INFINITE);

					if (Res == WAIT_OBJECT_0)
					{
						closesocket(ThreadInputs[i]);
						CloseHandle(ThreadHandles[i]);
						ThreadHandles[i] = NULL;
						break;
					}
					else printf("Waiting for thread failed. Ending program\n");					//better response - terminate brutally
					return;
				}
			}
		}

		printf("Client Connected.\n");

		for (i = 0; i < NUM_OF_WORKER_THREADS; i++)
		{
			if (ThreadHandles[i] == NULL)
				break;
			else
			{
				// poll to check if thread finished running:
				DWORD Res = WaitForSingleObject(ThreadHandles[i], 0);

				if (Res == WAIT_OBJECT_0) // this thread finished running
				{
					CloseHandle(ThreadHandles[i]);
					ThreadHandles[i] = NULL;
					break;
				}
			}
		}

		if (i == NUM_OF_WORKER_THREADS) //no slot is available
		{
			printf("No slots available for client, dropping the connection.\n");
			// send a message on the socket that connection was denied----------------------------------------------------------------------------//
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		}
		else
		{
			ThreadInputs[i] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // ThreadInputs[i] when the
											  // time comes.
			
			if (0 == i)
			{
				pData_0->ThreadInputs = ThreadInputs;
				pData_0->p_mutex_file = &mutex_file;
				pData_0->p_event = &event_two_players;
				pData_0->file_handle = &file_handle;
				pData_0->tasks_file_name = game_file_name;
				pData_0->thread_id = i;
				pData_0->stage_of_game = p_stage_of_game;
				pData_0->p_number_of_clients_connected = p_number_of_clients_connected;

				ThreadHandles[i] = CreateThread(
					NULL,
					0,
					ServiceThread,
					pData_0,
					0,
					&dwThreadId[i]
				);
			}

			else if (1 == i)
			{
				pData_1->ThreadInputs = ThreadInputs;
				pData_1->p_mutex_file = &mutex_file;
				pData_1->p_event = &event_two_players;
				pData_1->file_handle = &file_handle;
				pData_1->tasks_file_name = game_file_name;
				pData_1->thread_id = i;
				pData_1->stage_of_game = p_stage_of_game;
				pData_1->p_number_of_clients_connected = p_number_of_clients_connected;

				ThreadHandles[i] = CreateThread(
					NULL,
					0,
					ServiceThread,
					pData_1,
					0,
					&dwThreadId[i]
				);
			}

			//In case of error
			if (NULL == ThreadHandles[i])
			{
				printf("Brutally terminating the program due to thread dispatch issues");
				if (STATUS_CODE_FAILURE == (wait_for_threads_execution_and_free(ThreadHandles, ThreadInputs, event_two_players, TwoPlayerEventThread)))
				{
					printf("There was a wait error while we waited for all of the threads");
					return STATUS_CODE_FAILURE;
				}
				return STATUS_CODE_FAILURE;
			}
			//Else
			printf("dispatched sending Thread\n");

		}
	} 	//(while no quit)

		// escorting the threads for exit
	if (STATUS_CODE_FAILURE == (wait_for_threads_execution_and_free(ThreadHandles, ThreadInputs, event_two_players, TwoPlayerEventThread)))
	{
		printf("There was a wait error while we waited for all of the threads");
		return STATUS_CODE_FAILURE;
	}

	// Success
	printf("freed all threads\n");
	if (closesocket(MainSocket) == SOCKET_ERROR) printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
	if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
	return SUCCESS_CODE;

}



			

