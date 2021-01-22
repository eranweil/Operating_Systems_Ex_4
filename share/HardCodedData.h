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

// Personal client codes
#define I_START                                 21
#define I_QUIT                                  22
#define I_FAIL                                  23

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

// Game result codes
#define GAME_CONTINUES                          0
#define GAME_WON                                1
#define GAME_DRAW                               2

// Constants
#define SERVER_ADDRESS_STR                      "127.0.0.1"
#define NUM_OF_WORKER_THREADS                   2
#define MAX_LOOPS                               3
#define MAX_BYTES_SERVER_MIGHT_SEND             66
#define MAX_BYTES_CLIENT_MIGHT_SEND             27         
#define THREAD_WAIT_TIME                        15000


// Structures
struct lock
{
    HANDLE* p_mutex_file;
    HANDLE* p_event;
};
typedef struct lock LOCK;

struct Thread
{
    SOCKET* ThreadInputs;
    HANDLE* p_mutex_file;
    HANDLE* p_event;
    char* tasks_file_name;
    int thread_id;
    int* stage_of_game;
};
typedef struct Thread THREAD;


