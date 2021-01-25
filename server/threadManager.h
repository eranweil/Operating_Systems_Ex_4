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


DWORD WINAPI TwoPlayerEventMonitor(LPVOID lpParam);
