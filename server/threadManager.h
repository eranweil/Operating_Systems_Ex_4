#pragma once
/*
threadManager.h
----------------------------------------------------------------------------
All thread related actions header file
*/

//-------------------------------------------------------------//
// --------------------------INCLUDE-------------------------- //
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

#include "SocketSendRecvTools.h"
#include "HardCodedData.h"
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

