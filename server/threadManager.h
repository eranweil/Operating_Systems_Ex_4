#pragma once
/*
threadManager.h
----------------------------------------------------------------------------
All thread related actions header file
*/

//-------------------------------------------------------------//
// --------------------------INCLUDE-------------------------- //
//-------------------------------------------------------------//

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
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
DESCRIPTION - A function to break an integer into primes

PARAMETERS -    n - The integer to be broken down to primes
                p_prime_numbers - a pointer to an array of integers where the primes will be stored
                p_prime_numbers_size - a pointer to an outside variable where we get the number of primes. Will be updated in function for use of the calling function

RETURN - a pointer to an array of integers where the primes are stored
    --------------------------------------------------------------------------------------------*/
int* break_into_primes(int n, int* p_prime_numbers, int* p_prime_numbers_size);

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
int dispatch_threads(HANDLE* p_threads, LOCK* p_lock, QUEUE* p_queue, int number_of_threads, int* p_number_of_tasks, HANDLE* start_line_sephamore, char* tasks_file_name);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Function every new thread is called to. reads a task from the task file, breaks into primes and prints the correct string to the tasks file. uses a lock regiment as specified

PARAMETERS - lpParam: holds the data structure of pData for that thread

RETURN - signal exit code.
    --------------------------------------------------------------------------------------------*/
DWORD WINAPI thread_main(LPVOID lpParam);