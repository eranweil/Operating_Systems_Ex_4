#pragma once
/*
fileManager.h
----------------------------------------------------------------------------
All file an IO related actions header file
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

#include "HardCodedData.h"

//---------------------------------------------------------------//
// -------------------------DECLARATIONS------------------------ //
//---------------------------------------------------------------//

/*--------------------------------------------------------------------------------------------
DESCRIPTION - Making sure we got the right arguments in the command line

PARAMETERS -    argc - an integer with the number of command line arguments given
                argv[] - an array of strings holding the command line arguments

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int check_arguments(int argc, char* argv[]);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - A simple function for counting the digits of an integer

PARAMETERS - an integer

RETURN - number of digits
    --------------------------------------------------------------------------------------------*/
int CountDigits(int number);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - A function to get the task from the task file

PARAMETERS -    tasks_file_name - a string for the WINAPI file handle open function
                curr_task_num - an integer with the number of the current task in the queue. used for printouts and debug
                number_of_digits - a pointer to the number of digits. This gets updated during the function for use of the calling function

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int GetTask(char* tasks_file_name, int curr_task_num, int* number_of_digits);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - This is a function to create a string with the concatenated primes and with commas between, finishing with a \r\n

PARAMETERS -    string_prime - an unitialized char pointer which will hold the full string
                string_buffer - a buffer string for the creation of each individual prime
                prime_numbers_size - the number of primes for the inner loops
                number_of_digits - number of digits of the task. helpful for memory allocation
                prime_numbers - The array of integers which holds the prime numbers

RETURN - a pointer to the cancatenated string. memory allocated in this function needs to be released outside
    --------------------------------------------------------------------------------------------*/
char* MakePrimeString(char* string_prime, char* string_buffer, int prime_numbers_size, int number_of_digits, int* prime_numbers);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - This is a function prints the desired string to the tasks file

PARAMETERS -    tasks_file_name - used for the WINAPI file write handle function
                prime_numbers - The array of integers which holds the prime numbers
                prime_numbers_size - the number of primes
                n - the original task number
                number_of_digits - number of digits of the task. helpful for memory allocation

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int print_primes_to_file(char* tasks_file_name, int* prime_numbers, int prime_numbers_size, int n, int number_of_digits);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - The actual printer to file function

PARAMETERS -    tasks_file - a handle to the tasks file
                string - the string to print
                string_len - the length of the string for a safe print

RETURN - success code upon success or failure code otherwise
    --------------------------------------------------------------------------------------------*/
int StringToFileWithCheck(HANDLE tasks_file, char* string, int string_len);

/*--------------------------------------------------------------------------------------------
DESCRIPTION - A simplified function to open the file with the desired access

PARAMETERS -    lpFileName - name of the file to open
                dwDesiredAccess - type of access desired
                dwShareMode - type of sharing required
                dwCreationDisposition - disposition required

RETURN - Handle to the open file
    --------------------------------------------------------------------------------------------*/
HANDLE GetFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition);


