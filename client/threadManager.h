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

#include "Lock.h"
#include "fileManager.h"
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
void WaitError(DWORD wait_res, int thread_num);

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


void DefineStringToSend(THREAD* thread_params, char string_received[], char string_to_send[]);


/*--------------------------------------------------------------------------------------------
DESCRIPTION - Function every new thread is called to. reads a task from the task file, breaks into primes and prints the correct string to the tasks file. uses a lock regiment as specified

PARAMETERS - lpParam: holds the data structure of pData for that thread

RETURN - signal exit code.
    --------------------------------------------------------------------------------------------*/
DWORD WINAPI RecvDataThread(LPVOID lpParam);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Function every new thread is called to. reads a task from the task file, breaks into primes and prints the correct string to the tasks file. uses a lock regiment as specified

PARAMETERS - lpParam: holds the data structure of pData for that thread

RETURN - signal exit code.
    --------------------------------------------------------------------------------------------*/
DWORD WINAPI SendDataThread(LPVOID lpParam);