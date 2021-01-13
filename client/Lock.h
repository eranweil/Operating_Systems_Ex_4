#pragma once
/*
Lock.h
----------------------------------------------------------------------------
Lock module with all lock related actions header file
*/

//-------------------------------------------------------------//
// ----------------------LIBRARY INCLUDES--------------------- //
//-------------------------------------------------------------//

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdbool.h>

//-------------------------------------------------------------//
// ----------------------PROJECT INCLUDES--------------------- //
//-------------------------------------------------------------//

#include "HardCodedData.h"
#include "threadManager.h"

//-------------------------------------------------------------//
// ------------------FUNCTION DECLARATION--------------------- //
//-------------------------------------------------------------//

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Initializing the locks

PARAMETERS -    lock - pointer to the lock type element which was created outside the function
                number_of_threads - number of threads with is a field for the lock
                p_reader_counter - a pointer to a counter where we know how many readers are in the file
                p_mutex_file - a pointer to a mutex (mostly for read)
                p_sephamore_file - a pointer to a semaphore (mostly for write)
                p_turnstile - a pointer to a mutex designed to stop starvation for write actions
                p_queue_lock - A lock to the queue so that several threads don't take the same task

RETURN - a pointer to the initialized lock (or NULL if failed)
    --------------------------------------------------------------------------------------------*/
LOCK* InitializeLock(LOCK* lock, int number_of_threads, int* p_reader_counter, HANDLE* p_mutex_file, HANDLE* p_sephamore_file, HANDLE* p_turnstile, HANDLE* p_queue_lock);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Locking for read

PARAMETERS -    lock - pointer to the lock type element which all the threads are using

RETURN - either WAIT_OBJECT_0 if succeeded, or some other DWORD otherwise
    --------------------------------------------------------------------------------------------*/
DWORD read_lock(LOCK* lock);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - releasing after read

PARAMETERS -    lock - pointer to the lock type element which all the threads are using

RETURN - either FALSE if failed, or nonzero otherwise
    --------------------------------------------------------------------------------------------*/
BOOL read_release(LOCK* lock);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Locking for write

PARAMETERS -    lock - pointer to the lock type element which all the threads are using

RETURN - either WAIT_OBJECT_0 if succeeded, or some other DWORD otherwise
    --------------------------------------------------------------------------------------------*/
DWORD write_lock(LOCK* lock);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - releasing after write

PARAMETERS -    lock - pointer to the lock type element which all the threads are using

RETURN - either FALSE if failed, or nonzero otherwise
    --------------------------------------------------------------------------------------------*/
BOOL write_release(LOCK* lock);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Locking for queue

PARAMETERS -    lock - pointer to the lock type element which all the threads are using

RETURN - either WAIT_OBJECT_0 if succeeded, or some other DWORD otherwise
    --------------------------------------------------------------------------------------------*/
DWORD queue_lock(LOCK* lock);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - releasing after queue

PARAMETERS -    lock - pointer to the lock type element which all the threads are using

RETURN - either FALSE if failed, or nonzero otherwise
    --------------------------------------------------------------------------------------------*/
BOOL queue_release(LOCK* lock);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - releasing all of the memory allocated for the lock

PARAMETERS -    lock - pointer to the lock type element which all the threads are using

RETURN - a NULL pointer to a lock element
    --------------------------------------------------------------------------------------------*/
LOCK* DestroyLock(LOCK* lock);