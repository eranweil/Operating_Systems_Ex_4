
/*
threadManager.c
----------------------------------------------------------------------------
All thread related actions
*/

//-------------------------------------------------------------//
// --------------------------INCLUDE-------------------------- //
//-------------------------------------------------------------//

#include "threadManager.h"

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

//---------------------------------------------------------------//
// ------------------------IMPLEMENTAIONS------------------------//
//---------------------------------------------------------------//

void WaitError(DWORD wait_res, int thread_num)
{
    printf("When executing thread number %d ", thread_num);
    switch (wait_res)
    {
        case WAIT_ABANDONED:
        {
            printf("there was an abandoned sync element\n");
            break;
        }
        case WAIT_TIMEOUT:
        {
            printf("there was a sync element for which we waited too long\n");
            break;
        }
        case WAIT_FAILED:
        {
            printf("there was a failed sync element\n");
            break;
        }
        default:
        {
            printf("there was an unrecognized sync element issue\n");
            break;
        }
    }
}

int* break_into_primes(int n, int* p_prime_numbers, int* p_prime_numbers_size)
{
    size_t prime_numbers_index = *p_prime_numbers_size;
    int prime_number = n;
    int i = 3;
    if (NULL == (p_prime_numbers = (int*)malloc(prime_numbers_index * sizeof(int)))) return p_prime_numbers;

    while ((prime_number % 2) == 0)
    {
        *(p_prime_numbers + prime_numbers_index - 1) = 2;
        prime_number = prime_number / 2;
        printf("Number %d of prime %d is %d\n", prime_numbers_index, n, 2);
        prime_numbers_index++;
        if (NULL == (p_prime_numbers = (int*)realloc(p_prime_numbers, prime_numbers_index * sizeof(int)))) return p_prime_numbers;
    }

    while (i <= sqrt(prime_number))
    {
        while ((prime_number % i) == 0)
        {
            *(p_prime_numbers + prime_numbers_index - 1) = i;
            prime_number = prime_number / i;
            printf("Number %d of prime %d is %d\n", prime_numbers_index, n, i);
            prime_numbers_index++;
            if (NULL == (p_prime_numbers = (int*)realloc(p_prime_numbers, prime_numbers_index * sizeof(int)))) return p_prime_numbers;
        }
        i = i + 2;
    }

    if (prime_number > 2)
    {
        *(p_prime_numbers + prime_numbers_index - 1) = prime_number;
        printf("Number %d of prime %d is %d\n", prime_numbers_index, n, prime_number);
    }
    else prime_numbers_index--;
    *p_prime_numbers_size = prime_numbers_index;
    return p_prime_numbers;
}

int wait_for_threads_execution_and_free(HANDLE* p_threads, int number_of_threads)
{
    int i = 0;
    DWORD dwEvent;

    dwEvent = WaitForMultipleObjects(
        number_of_threads,                                          // number of objects in array
        p_threads,                                                  // array of objects
        TRUE,                                                       // wait for all object
        10 * number_of_threads * THREAD_WAIT_TIME);                 // long wait period for all threads
    
    if (WAIT_OBJECT_0 != dwEvent)
    {
        // Print Error message
        WaitError(dwEvent, 0); 
        // Free the threads which were dispatched, because some might have been
        for (i = 0; i < number_of_threads; i++)
        {
            if (NULL != (p_threads + i))
            {
                CloseHandle(*(p_threads + i));
                printf("freed Thread number: %d\n", i + 1);
            }
        }
        return STATUS_CODE_FAILURE;
    }
    // Close event handles in the event of a total success
    for (i = 0; i < number_of_threads; i++)
    {
        CloseHandle(*(p_threads + i));
        printf("freed Thread number: %d\n", i + 1);
    }
    return SUCCESS_CODE;
}

int dispatch_threads(HANDLE* p_threads, LOCK* p_lock, QUEUE* p_queue, int number_of_threads, int* p_number_of_tasks, HANDLE* p_start_line_sephamore, char* tasks_file_name)
{
    int i = 0, j = 0;

    THREAD Data;
    THREAD* pData = &Data;
    DWORD dwThreadId;


    pData->p_lock = p_lock;
    pData->p_queue = p_queue;
    pData->p_number_of_tasks = p_number_of_tasks;
    pData->start_line_sephamore = p_start_line_sephamore;
    pData->tasks_file_name = tasks_file_name;

    // continue dispatching threads until 0 threads are needed
    for (i = 0; i < number_of_threads; i++)
    {
        pData->thread_number = i + 1;

        // create thread
        *(p_threads + i) = CreateThread(
            NULL,                                           // default security attributes
            0,                                              // use default stack size  
            thread_main,                                    // thread function name
            pData,                                          // argument to thread function 
            0,                                              // use default creation flags 
            &dwThreadId);                                   // returns the thread identifier 

        //In case of error
        if (NULL == *(p_threads + i))
        {
            printf("Brutally terminating the program due to thread dispatch issues");
            //When one thread doesn't succeed free all those that did before it
            for (j = 0; j < i; j++)
            {
                CloseHandle(*(p_threads + j));
            }
            return STATUS_CODE_FAILURE;
        }
        //Else
        printf("dispatched Thread number: %d\n", i + 1);

    }

    // Release start line Sephamore
    if (FALSE == (ReleaseSemaphore(*p_start_line_sephamore, number_of_threads, NULL)))
    {
        printf("Couldn't release start line semaphore");
        return STATUS_CODE_FAILURE;
    }
    printf("start line semaphore released\n");

    // escorting the threads for exit
    if (STATUS_CODE_FAILURE == (wait_for_threads_execution_and_free(p_threads, number_of_threads)))
    {
        printf("There was a wait error while we waited for all of the threads");
        return STATUS_CODE_FAILURE;
    }

    // Success
    printf("freed all threads\n");
    return SUCCESS_CODE;
}

DWORD WINAPI thread_main(LPVOID lpParam)
{
    
    DWORD wait_res;
    BOOL release_res;
    THREAD* thread_params = (THREAD*)lpParam;
    int curr_task_num = 0, n = 0, number_of_digits = 0, prime_numbers_size = 1, thread_num = thread_params->thread_number;
    int* p_prime_numbers = NULL;
    int* p_number_of_digits = &number_of_digits, p_prime_numbers_size = &prime_numbers_size;
    char* line = NULL;

    // Wait at the starting line for all threads to be created
    if (WAIT_OBJECT_0 != (wait_res = (WaitForSingleObject(*(thread_params->start_line_sephamore), thread_params->p_lock->number_of_threads * THREAD_WAIT_TIME))))
    {
        WaitError(wait_res, thread_num);
        return STATUS_CODE_FAILURE;
    }

    // Run as long as there are more tasks left
    while ((*(thread_params->p_number_of_tasks)) > 0)
    {
        p_prime_numbers = NULL;

        // Get the next prioritized task
        if (WAIT_OBJECT_0 != (wait_res = (queue_lock(thread_params->p_lock))))
        {
            WaitError(wait_res, thread_num);
            return STATUS_CODE_FAILURE;
        }
        curr_task_num = Pop(thread_params->p_queue)->priority;
        *(thread_params->p_number_of_tasks) -= 1;
        if (FALSE == (release_res = queue_release(thread_params->p_lock))) return STATUS_CODE_FAILURE;
        printf("Thread %d finished with the queue\n", thread_num);

        // Get the task
        if (WAIT_OBJECT_0 != (wait_res = (read_lock(thread_params->p_lock))))
        {
            WaitError(wait_res, thread_num);
            return STATUS_CODE_FAILURE;
        }
        if (STATUS_CODE_FAILURE == (n = GetTask(thread_params->tasks_file_name, curr_task_num, p_number_of_digits))) return STATUS_CODE_FAILURE;
        if (FALSE == (release_res = read_release(thread_params->p_lock))) return STATUS_CODE_FAILURE;
        printf("Thread %d finished reading task number: %d\n", thread_num, *(thread_params->p_number_of_tasks));

        // Calculate the prime numbers
        if (NULL == (p_prime_numbers = break_into_primes(n, p_prime_numbers, p_prime_numbers_size))) return STATUS_CODE_FAILURE;
        printf("Thread %d broke into primes\n", thread_num);

        // Write to the file
        if (WAIT_OBJECT_0 != (wait_res = (write_lock(thread_params->p_lock))))
        {
            WaitError(wait_res, thread_num);
            return STATUS_CODE_FAILURE;
        }
        print_primes_to_file(thread_params->tasks_file_name, p_prime_numbers, prime_numbers_size, n, number_of_digits);
        if (FALSE == (release_res = write_release(thread_params->p_lock))) return STATUS_CODE_FAILURE;
        p_prime_numbers = NULL;
        prime_numbers_size = 1;
        number_of_digits = 0;
        printf("Thread %d finished writing task number: %d\n", thread_num, *(thread_params->p_number_of_tasks));
        free(p_prime_numbers);
    }

    return SUCCESS_CODE;
}