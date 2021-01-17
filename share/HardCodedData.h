#pragma once
/*
HardCodedData.h
----------------------------------------------------------------------------
Contains specific measured sizes, exit codes and structures of the project. 
*/


//-------------------------------------------------------------//
// --------------------------DEFINES-------------------------- //
//-------------------------------------------------------------//
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>

// Exit codes
#define SUCCESS_CODE							0 
#define STATUS_CODE_FAILURE					   -1 
#define ERROR_CODE_FILE						   -2
#define ERROR_CODE_HANDLE					   -3
#define ERROR_CODE_ARGS						   -4
#define ERROR_CODE_ALLOCATION				   -5
#define ERROR_CODE_THREAD					   -6
#define ERROR_CODE_WSO_WAIT_ABANDONED		   -7
#define ERROR_CODE_WSO_WAIT_TIMEOUT			   -8
#define ERROR_CODE_WSO_WAIT_FAILED			   -9
#define _WINSOCK_DEPRECATED_NO_WARNINGS


// Bool
#define TRUE									1 
#define FALSE								    0

// Sizes
#define ARGS_REQUIRED_CLIENT                    4
#define ARGS_REQUIRED_SERVER                    2
#define THREAD_WAIT_TIME_IN_MILLISECONDS		3000
#define WAIT_FOR_SERVER_IN_MILLISECONDS         15000

// Client codes
#define CLIENT_REQUEST                          0
#define CLIENT_VERSUS                           1
#define CLIENT_SETUP                            2
#define CLIENT_PLAYER_MOVE                      3
#define CLIENT_DISCONNECT                       4

// Server codes
#define SERVER_MAIN_MENU                        0
#define SERVER_APPROVED                         1
#define SERVER_INVITE                           2
#define SERVER_SETUP_REQUEST                    3
#define SERVER_PLAYER_MOVE_REQUEST              4
#define SERVER_GAME_RESULTS                     5
#define SERVER_WIN                              6
#define SERVER_DRAW                             7
#define SERVER_NO_OPPONENTS                     8
#define SERVER_OPPONENT_QUIT                    9
#define SERVER_DENIED                           10
#define MAX_BYTES_SERVER_MIGHT_SEND             66

// Personal client codes
#define I_START                                 -1
#define I_QUIT                                  -2
#define I_FAIL                                  -3


// Structures
struct lock
{
    int number_of_threads;
    int* p_reader_counter;
    HANDLE* p_mutex_file;
    HANDLE* p_sephamore_file;
    HANDLE* p_turnstile;
    HANDLE* p_queue_lock;
};
typedef struct lock LOCK;

struct Thread
{
    SOCKET*	m_socket;
    LOCK* p_lock;
    int* receive_thread_result;
    //HANDLE* start_line_sephamore;
};
typedef struct Thread THREAD;


