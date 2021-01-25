/*
main.c
----------------------------------------------------------------------------
AUTHORS - Itamar Eyal 302309539 and Eran Weil 204051353

PROJECT - Client

DESCRIPTION - This program generates the client side in the Bulls and Cows game


    consists of 2 main modules:
        threadManager - creates and handles threads and thread functions
        SocketSendRecvTools - A module for all Socket connection and data transfer
*/

//-------------------------------------------------------------//
// ----------------------PROJECT INCLUDES--------------------- //
//-------------------------------------------------------------//

#include "HardCodedData.h"
#include "threadManager.h"

int check_arguments(int argc, char** argv)
{
    // making sure there are enough args given
    if (argc - 1 < ARGS_REQUIRED_CLIENT)
    {
        printf("Too few arguments given. We need %d arguments and you have provided %d\n", ARGS_REQUIRED_CLIENT, argc - 1);
        return STATUS_CODE_FAILURE;
    }

    if (argc - 1 > ARGS_REQUIRED_CLIENT)
    {
        printf("Too many arguments given. We need %d arguments and you have provided %d\n", ARGS_REQUIRED_CLIENT, argc - 1);
        return STATUS_CODE_FAILURE;
    }

    if (argc - 1 == ARGS_REQUIRED_CLIENT)
    {
        printf("The IP number is %s\n", *(argv + 1));
        printf("The port number is %s\n", *(argv + 2));
        printf("The client name is %s\n", *(argv + 3));
    }
    return SUCCESS_CODE;
}


int main(int argc, char** argv)
{
    char        user_input[] = "00000000000000000000",        //21
                opponent_username[] = "00000000000000000000", //21
                winners_name[] = "00000000000000000000",      //21
                opponent_guess[] = "0000",                    //5
                bulls[] = "0",                                //2
                cows[] = "0",                                 //2
                received_string[] = "00000000000000000000000000000000000000000000000000000000000000000", //66
                server_ip[]= "000000000000000",               //16
                server_port[] = "00000",                       //5
                client_name[] = "00000000000000000000",       //21
                send_string[37];


                strcpy_s(server_ip, strlen(server_ip), *(argv + 1));
                strcpy_s(server_port, strlen(server_port), *(argv + 2));
                strcpy_s(client_name, strlen(client_name), *(argv + 3));

    int		    server_port_num = atoi(server_port),
                game_state = I_QUIT,
                opponent_name_len = 0;

    int*        p_game_state = &game_state;
    int*        p_opponent_name_len = &opponent_name_len;

	SOCKET	    m_socket;
	SOCKADDR_IN clientService;

    // Check args given
    if (STATUS_CODE_FAILURE == check_arguments(argc, argv)) return ERROR_CODE_ARGS;

    // Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (StartupRes != NO_ERROR)
    {
        printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
        // Tell the user that we could not find a usable WinSock DLL.                                  
        return;
    }

	// Create a socket and check for errors to ensure that the socket is a valid socket.
	if ((m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) 
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	// Connect to a server.

	// Create a sockaddr_in object clientService and set  values.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(server_ip); //Setting the IP address to connect to
	clientService.sin_port = htons(server_port_num); //Setting the port to connect to.

	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
    
    game_state = I_START;

    if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) game_state = I_FAIL;
    
    while (game_state != I_QUIT)
    {
        if (STATUS_CODE_FAILURE == GameState(&m_socket, &clientService, p_game_state, user_input, send_string, received_string, server_ip, server_port, p_opponent_name_len, winners_name, opponent_username, bulls, cows, opponent_guess)) game_state = I_FAIL;
    }

    //Socket closed inside the game - no exiting game without closing socket
    if (WSACleanup() == SOCKET_ERROR) printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
    return SUCCESS_CODE;
}