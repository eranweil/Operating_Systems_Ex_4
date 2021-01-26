

/*
main.c
----------------------------------------------------------------------------
AUTHORS - Itamar Eyal 302309539 and Eran Weil 204051353

PROJECT - Server

DESCRIPTION - This program functions as a server to clients in the BUlls and Cows game


	consists of 4 main modules:
		game - actual functionality of the game
		threadManager - creates and handles threads and thread functions
		SocketSendRecvTools - A module for all Socket connection and data transfer
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

/*--------------------------------------------------------------------------------------------
DESCRIPTION -check that user has given the correct number of arguments

PARAMETERS - args

RETURN - success code upon success or failure code otherwise
	--------------------------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------------------------
DESCRIPTION - function for initializing main socket

PARAMETERS - pointer to main socket and args given (for port)

RETURN - success code upon success or failure code otherwise
	--------------------------------------------------------------------------------------------*/
int InitializeMainSocket(SOCKET* MainSocket, char** argv)
{
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	int i = 0;
	char server_ip[] = "127.0.0.1";
	char server_port[] = "8888";
	for (i = 0; i < 4; i++) server_port[i] = *((*(argv + 1)) + i);
	int server_port_num = atoi(server_port);



	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return STATUS_CODE_FAILURE;
	}
	/* The WinSock DLL is acceptable. Proceed. */

	// Create a socket.    
	if ((*MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return STATUS_CODE_FAILURE;
	}

	// Bind the socket.
	if ((Address = inet_addr(server_ip)) == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n", server_ip);
		if (closesocket(*MainSocket) == SOCKET_ERROR)
		{
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
			if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
			return STATUS_CODE_FAILURE;
		}
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(server_port_num);

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	if ((bindRes = bind(*MainSocket, (SOCKADDR*)&service, sizeof(service))) == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		if (closesocket(*MainSocket) == SOCKET_ERROR)
		{
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
			if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
			return STATUS_CODE_FAILURE;
		}
	}

	// Listen on the Socket.
	if ((ListenRes = listen(*MainSocket, SOMAXCONN)) == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		if (closesocket(*MainSocket) == SOCKET_ERROR)
		{
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
			if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
			return STATUS_CODE_FAILURE;
		}
	}
	return SUCCESS_CODE;
}

/*--------------------------------------------------------------------------------------------
DESCRIPTION - function for creating all the relevent events and threads

PARAMETERS - pointers to all the events and threads, as well as their necessary parameters

RETURN - success code upon success or failure code otherwise
	--------------------------------------------------------------------------------------------*/
int CreateEventsAndEventThreads(HANDLE* event_two_players, HANDLE* ExitEvent, HANDLE* event_player_2, HANDLE* TwoPlayerEventThread, HANDLE* ExitThread, TWO_PLAYER_THREAD* p_two_player_data, int* p_number_of_clients_connected)
{
	DWORD dwTwoPlayerEventThreadId;
	DWORD dwExitThreadId;
	char two_player_event_name[] = "TWO_PLAYER_EVENT";
	char event_player_2_name[] = "PLAYER_TWO_EVENT";
	char exit_event_name[] = "EXIT_EVENT";

	if (NULL == (*event_two_players = CreateEvent(
		NULL,					/* default security attributes */
		TRUE,					/* manual-reset event */
		FALSE,					/* initial state is signaled */
		two_player_event_name)))			/* name */
	{
		return STATUS_CODE_FAILURE;
	}


	if (NULL == (*ExitEvent = CreateEvent(
		NULL,					/* default security attributes */
		TRUE,					/* manual-reset event */
		FALSE,					/* initial state is signaled */
		exit_event_name)))			/* name */
	{
		CloseHandle(*event_two_players);
		return STATUS_CODE_FAILURE;
	}

	if (NULL == (*event_player_2 = CreateEvent(
		NULL,					/* default security attributes */
		FALSE,					/* manual-reset event */
		FALSE,					/* initial state is signaled */
		event_player_2_name)))			/* name */
	{
		CloseHandle(*event_two_players);
		CloseHandle(*ExitEvent);
		return STATUS_CODE_FAILURE;
	}

	p_two_player_data->p_number_of_clients_connected = p_number_of_clients_connected;
	p_two_player_data->p_event = event_two_players;
	p_two_player_data->p_ExitEvent = ExitEvent;

	if (NULL == (*TwoPlayerEventThread = CreateThread(
		NULL,
		0,
		TwoPlayerEventMonitor,
		p_two_player_data,
		0,
		&dwTwoPlayerEventThreadId
	)))
	{
		CloseHandle(*event_two_players);
		CloseHandle(*ExitEvent);
		CloseHandle(*event_player_2);
		return STATUS_CODE_FAILURE;
	}

	if (NULL == (*ExitThread = CreateThread(
		NULL,
		0,
		ExitMonitor,
		ExitEvent,
		0,
		&dwExitThreadId
	)))
	{
		CloseHandle(*event_two_players);
		CloseHandle(*ExitEvent);
		CloseHandle(*event_player_2);
		CloseHandle(*TwoPlayerEventThread);
		return STATUS_CODE_FAILURE;
	}
	return SUCCESS_CODE;
}

/*--------------------------------------------------------------------------------------------
DESCRIPTION - A small function to free the events and threads in case of errors, using their handles as input
	--------------------------------------------------------------------------------------------*/
int FreeEventAndThreadHandles(HANDLE* event_two_players, HANDLE* ExitEvent, HANDLE* event_player_2, HANDLE* TwoPlayerEventThread, HANDLE* ExitThread)
{
	if (FALSE == (CloseHandle(*event_two_players))) return STATUS_CODE_FAILURE;
	if (FALSE == (CloseHandle(*ExitEvent))) return STATUS_CODE_FAILURE;
	if (FALSE == (CloseHandle(*event_player_2))) return STATUS_CODE_FAILURE;
	if (FALSE == (CloseHandle(*TwoPlayerEventThread))) return STATUS_CODE_FAILURE;
	if (FALSE == (CloseHandle(*ExitThread))) return STATUS_CODE_FAILURE;
	return SUCCESS_CODE;
}


int main(int argc, char** argv)
{
	char game_file_name[] = "GameSession.txt";
	char server_denied_str[] = "SERVER_DENIED\n";
	int i = 0;
	int j = 0;
	int number_of_clients_connected = 0;
	int* p_number_of_clients_connected = &number_of_clients_connected;
	int mode_res = 0;

	HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	HANDLE TwoPlayerEventThread = NULL;
	HANDLE ExitThread = NULL;
	HANDLE ExitEvent = NULL;
	HANDLE file_handle = NULL;
	HANDLE mutex_file = NULL;
	HANDLE event_two_players = NULL;
	HANDLE event_player_2 = NULL;

	SOCKET MainSocket = INVALID_SOCKET;
	SOCKET AcceptSocket = INVALID_SOCKET;
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];

	THREAD Data_0;
	THREAD Data_1;
	THREAD* pData_0 = &Data_0;
	THREAD* pData_1 = &Data_1;
	TWO_PLAYER_THREAD two_player_data;
	TWO_PLAYER_THREAD* p_two_player_data = &two_player_data;
	DWORD dwThreadId[NUM_OF_WORKER_THREADS];

	u_long socket_non_blocking_mode = 1, socket_blocking_mode = 0;

	// Check args given
	if (STATUS_CODE_FAILURE == check_arguments(argc, argv)) return ERROR_CODE_ARGS;
	//Initialize main socket
	if (STATUS_CODE_FAILURE == (InitializeMainSocket(&MainSocket, argv))) return ERROR_WSA;
	//open all events and event threads
	if (STATUS_CODE_FAILURE == (CreateEventsAndEventThreads(&event_two_players, &ExitEvent, &event_player_2, &TwoPlayerEventThread, &ExitThread, p_two_player_data, p_number_of_clients_connected))) return ERROR_CODE_THREAD;
	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (i = 0; i < NUM_OF_WORKER_THREADS; i++) ThreadHandles[i] = NULL;

	printf("Waiting for a client to connect...\n");

	mode_res = ioctlsocket(MainSocket, FIONBIO, &socket_non_blocking_mode);
	if (SOCKET_ERROR == mode_res)
	{
		printf("ioctlsocket failed with error: %ld\n", mode_res);
		if (STATUS_CODE_FAILURE == (FreeEventAndThreadHandles(&event_two_players, &ExitEvent, &event_player_2, &TwoPlayerEventThread, &ExitThread))) return ERROR_CODE_THREAD;;
		return ERROR_WSA;
	}

	while (WAIT_OBJECT_0 != WaitForSingleObject(ExitEvent, 0))
	{
		AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			if (WSAEWOULDBLOCK == WSAGetLastError()) continue; // loop back until accept 

			//Real error
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
					else printf("Waiting for thread failed. Ending program\n");
					break;
				}
			}
		}

		//Blocking socket - it is a worker
		mode_res = ioctlsocket(AcceptSocket, FIONBIO, &socket_blocking_mode);
		if (SOCKET_ERROR == mode_res)
		{
			printf("ioctlsocket failed with error: %ld\n", mode_res);
			if (STATUS_CODE_FAILURE == (FreeEventAndThreadHandles(&event_two_players, &ExitEvent, &event_player_2, &TwoPlayerEventThread, &ExitThread))) return ERROR_CODE_THREAD;;
			return ERROR_WSA;
		}
		//Upon Success
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
			SendData(AcceptSocket, server_denied_str);
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
			continue;
		}
		else
		{
			ThreadInputs[i] = AcceptSocket;

			if (0 == i)
			{
				pData_0->ThreadInputs = ThreadInputs;
				pData_0->p_mutex_file = &mutex_file;
				pData_0->p_event = &event_two_players;
				pData_0->file_handle = &file_handle;
				pData_0->event_player_2 = &event_player_2;
				pData_0->p_ExitEvent = &ExitEvent;
				pData_0->tasks_file_name = game_file_name;
				pData_0->thread_id = i;
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
				pData_1->event_player_2 = &event_player_2;
				pData_1->p_ExitEvent = &ExitEvent;
				pData_1->tasks_file_name = game_file_name;
				pData_1->thread_id = i;
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
				break;
			}
			//Else
			printf("dispatched Thread\n");

		}
	} 	//(while no exit)

		// escorting all handles, sockets and threads for exit
	if (STATUS_CODE_FAILURE == (FreeAll(ThreadHandles, ThreadInputs, &event_two_players, &TwoPlayerEventThread, &ExitThread, &ExitEvent, &file_handle, &mutex_file, &event_player_2, &MainSocket, game_file_name)))
	{
		return STATUS_CODE_FAILURE;
	}

	// Success
	printf("freed all threads, closed all handles and Sockets\n");
	if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
	return SUCCESS_CODE;

}



			

