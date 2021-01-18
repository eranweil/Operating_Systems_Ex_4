

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
#include "SocketSendRecvTools.h"


int main(int argc, char** argv[])
{
	char server_ip[] = "127.0.0.1";
	char server_port[5] = "";
	char game_file_name[]= "GameSession.txt";
	strcpy(server_port, argv[1]);

	int i = 0;
	int j = 0;
	int stage_of_game = 0;
	int server_port_num = atoi(server_port);
	int* p_stage_of_game = &stage_of_game;

	HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];

	HANDLE mutex_file;
	HANDLE event_two_players;

	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	LOCK lock;
	LOCK* p_lock = &lock;
	THREAD Data;
	THREAD* pData = &Data;
	DWORD dwThreadId[NUM_OF_WORKER_THREADS];

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

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (i = 0; i < NUM_OF_WORKER_THREADS; i++) ThreadHandles[i] = NULL;

	printf("Waiting for a client to connect...\n");

	for (j = 0; j < MAX_LOOPS; j++)
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
					else
					{
						printf("Waiting for thread failed. Ending program\n");
						return;
					}
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
			
			pData->ThreadInputs = ThreadInputs;
			pData->p_mutex_file = &mutex_file;
			pData->p_event = &event_two_players;
			pData->tasks_file_name = game_file_name;
			pData->thread_id = i;
			pData->stage_of_game = p_stage_of_game;

			ThreadHandles[i] = CreateThread(
				NULL,
				0,
				ServiceThread,
				pData,
				0,
				&dwThreadId[i]
			);

			//In case of error
			if (NULL == ThreadHandles[i])
			{
				printf("Brutally terminating the program due to thread dispatch issues");
				if (STATUS_CODE_FAILURE == (wait_for_threads_execution_and_free(ThreadHandles, ThreadInputs)))
				{
					printf("There was a wait error while we waited for all of the threads");
					return STATUS_CODE_FAILURE;
				}
				return STATUS_CODE_FAILURE;
			}
			//Else
			printf("dispatched sending Thread\n");

		}
	} // for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )

		// escorting the threads for exit
	if (STATUS_CODE_FAILURE == (wait_for_threads_execution_and_free(ThreadHandles, ThreadInputs)))
	{
		printf("There was a wait error while we waited for all of the threads");
		return STATUS_CODE_FAILURE;
	}

	// Success
	printf("freed all threads\n");
	return SUCCESS_CODE;
	if (closesocket(MainSocket) == SOCKET_ERROR) printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
	if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());

}



			

