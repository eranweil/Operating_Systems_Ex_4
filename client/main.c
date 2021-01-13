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
#include "Lock.h"



int main(int argc, char** argv[])
{
    char*   server_ip = argv[1], 
            server_port = argv[2], 
            client_name = argv[3];

	int		server_port_num = atoi(server_port);
	SOCKET	m_socket;
	SOCKADDR_IN clientService;
	HANDLE hThread[2];
    HANDLE mutex_file, sephamore_file, turnstile, queue_lock, start_line_sephamore;


    LOCK lock;
    LOCK* p_lock = &lock;

	WSADATA wsaData;

    // Check args given
    if (STATUS_CODE_FAILURE == check_arguments(argc, argv)) return ERROR_CODE_ARGS;

	//Call WSAStartup and check for errors.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) printf("Error at WSAStartup()\n");

	// Create a socket and check for errors to ensure that the socket is a valid socket.
	if ((m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) 
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();																											//need to fix
		return;
	}

	// Connect to a server.

	// Create a sockaddr_in object clientService and set  values.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(server_ip); //Setting the IP address to connect to
	clientService.sin_port = htons(server_port_num); //Setting the port to connect to.

	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) 
	{
		printf("Failed to connect.\n");
		WSACleanup();
		return;
	}


    //// Create the lock
    //if (NULL == (p_lock = InitializeLock(p_lock, number_of_threads, p_reader_counter, &mutex_file, &sephamore_file, &turnstile, &queue_lock)))
    //{
    //    printf("There was trouble initializing the queue");
    //    return ERROR_CODE_ALLOCATION;
    //}
    //printf("The lock was created\n");

    //// Create Sephamore to hold back all of the threads
    //if (NULL == (start_line_sephamore = CreateSemaphore(NULL, 0, number_of_threads, sephamore_start_line)))
    //{
    //    printf("Start line semaphore was not created\n");
    //    return ERROR_CODE_HANDLE;
    //}
    //printf("start line semaphore created\n");

    // Create the treads and give them tasks
    if (STATUS_CODE_FAILURE == (dispatch_threads(hThread, &m_socket, p_lock)))
    {
        printf("There was a problem dispatching threads\n");
        return ERROR_CODE_THREAD;
    }
    printf("all threads finished\n");

    // Get rid of all left over memory and handles
    DestroyLock(p_lock);

    return SUCCESS_CODE;
}