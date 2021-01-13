
/*
Lock.c
----------------------------------------------------------------------------
Lock module with all lock related actions
*/

//-------------------------------------------------------------//
// --------------------------INCLUDE-------------------------- //
//-------------------------------------------------------------//

#include "Lock.h"

//---------------------------------------------------------------//
// -------------------------DECLARATIONS------------------------ //
//---------------------------------------------------------------//

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

//---------------------------------------------------------------//
// ------------------------IMPLEMENTAIONS------------------------//
//---------------------------------------------------------------//


LOCK* InitializeLock(LOCK* lock, int number_of_threads, int* p_reader_counter, HANDLE* p_mutex_file, HANDLE* p_sephamore_file, HANDLE* p_turnstile, HANDLE* p_queue_lock)
{
    *p_mutex_file = CreateMutex(NULL, FALSE, NULL);
    if (p_mutex_file == NULL)
    {
        return lock;
    }
    *p_sephamore_file = CreateSemaphore(NULL, 1, (LONG)number_of_threads, NULL);
    if (p_sephamore_file == NULL)
    {
        CloseHandle(*p_mutex_file);
        return lock;
    }    
    *p_turnstile = CreateMutex(NULL, FALSE, NULL);
    if (p_turnstile == NULL)
    {
        CloseHandle(*p_mutex_file);
        CloseHandle(*p_sephamore_file);
        return lock;
    }
    *p_queue_lock = CreateMutex(NULL, FALSE, NULL);
    if (p_queue_lock == NULL) 
    {
        CloseHandle(*p_mutex_file);
        CloseHandle(*p_sephamore_file);
        CloseHandle(*p_turnstile);
        return lock;
    }
    lock->number_of_threads = number_of_threads;
    lock->p_reader_counter = p_reader_counter;
    lock->p_mutex_file = p_mutex_file;
    lock->p_sephamore_file = p_sephamore_file;
    lock->p_turnstile = p_turnstile;
    lock->p_queue_lock = p_queue_lock;

    return lock;
}

DWORD read_lock(LOCK* lock)
{
    DWORD wait_res;
    BOOL release_res;

    //Turnstile
    if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*(lock->p_turnstile), lock->number_of_threads * THREAD_WAIT_TIME))) return wait_res;

    if (FALSE == (release_res = ReleaseMutex(*(lock->p_turnstile))))
    {
        printf("Couldn't release Mutex");
        return wait_res;
    }

    //Mutex
    if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*(lock->p_mutex_file), lock->number_of_threads * THREAD_WAIT_TIME))) return wait_res;

    *lock->p_reader_counter += 1;
    if (*lock->p_reader_counter == 1)
    {
        if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*lock->p_sephamore_file, lock->number_of_threads * THREAD_WAIT_TIME))) return wait_res;
    }
    if (FALSE == (release_res = ReleaseMutex(*(lock->p_mutex_file))))
    {
        printf("Couldn't release Mutex");
        return wait_res;
    }
    return wait_res;
}

BOOL read_release(LOCK* lock)
{
    DWORD wait_res;
    BOOL release_res;

    //Mutex
    if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*(lock->p_mutex_file), lock->number_of_threads * THREAD_WAIT_TIME))) return FALSE;
    *lock->p_reader_counter -= 1;
    if (0 == *lock->p_reader_counter)
    {   
        if (FALSE == (release_res = ReleaseSemaphore(*(lock->p_sephamore_file), 1, NULL))) printf("Couldn't release Mutex");
    }
    if (FALSE == (release_res = ReleaseMutex(*(lock->p_mutex_file)))) printf("Couldn't release Mutex");
    return release_res;
}

DWORD write_lock(LOCK* lock)
{
    DWORD wait_res;
    BOOL release_res;

    //Turnstile
    if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*(lock->p_turnstile), lock->number_of_threads * THREAD_WAIT_TIME))) return wait_res;
    
    //Semaphore
    if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*(lock->p_sephamore_file), lock->number_of_threads * THREAD_WAIT_TIME))) return wait_res;

}

BOOL write_release(LOCK* lock)
{
    BOOL release_res;
    if (FALSE == (release_res = ReleaseMutex(*(lock->p_turnstile)))) printf("Couldn't release Mutex");
    if (FALSE == (release_res = ReleaseSemaphore(*(lock->p_sephamore_file), 1, NULL))) printf("Couldn't release Semaphore");
    return release_res;
}

DWORD queue_lock(LOCK* lock)
{
    DWORD wait_res;
    if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(*(lock->p_queue_lock), lock->number_of_threads * THREAD_WAIT_TIME))) return wait_res;
}

BOOL queue_release(LOCK* lock)
{
    BOOL release_res;
    if (FALSE == (release_res = ReleaseMutex(*(lock->p_queue_lock)))) printf("Couldn't release queue lock\n");
    return release_res;
}

LOCK* DestroyLock(LOCK* lock)
{
    CloseHandle(*lock->p_mutex_file);
    CloseHandle(*lock->p_sephamore_file);
    CloseHandle(*lock->p_turnstile);
    CloseHandle(*lock->p_queue_lock);
    return (lock = NULL);
}
