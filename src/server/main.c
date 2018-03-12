#include <SDL/SDL_net.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../shared/utils.h"

#define SDL_FLAGS 0

#define SERVER_PORT 1000
#define MAX_SOCKETS 2
#define PACKET_SIZE 256

typedef struct tcp_client_s
{
    int id;
    TCPsocket socket;
} tcp_client_t;

int get_client_id(void);

tcp_client_t tcp_clients[MAX_SOCKETS];

int main(int argc, char *args[])
{
    // suppress warning C4100
    (void)argc;
    (void)args;

    if (SDL_Init(SDL_FLAGS) == -1)
    {
        SDL_Log("SDL_Init: %s", SDL_GetError());

        return 1;
    }

    if (SDLNet_Init() != 0)
    {
        SDL_Log("SDLNet_Init: %s", SDLNet_GetError());

        return 1;
    }

    IPaddress ip;

    if (SDLNet_ResolveHost(&ip, NULL, SERVER_PORT) != 0)
    {
        SDL_Log("SDLNet_ResolveHost: %s", SDLNet_GetError());

        return 1;
    }

    TCPsocket tcp_server = SDLNet_TCP_Open(&ip);

    if (!tcp_server)
    {
        SDL_Log("SDLNet_TCP_Open: %s", SDLNet_GetError());

        return 1;
    }

    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        tcp_clients[i].id = -1;
        tcp_clients[i].socket = NULL;
    }

    SDLNet_SocketSet sockets = SDLNet_AllocSocketSet(MAX_SOCKETS);

    if (!sockets)
    {
        SDL_Log("SDLNet_AllocSocketSet: %s", SDLNet_GetError());

        return 1;
    }

    SDL_Log("Listening on port %d", SERVER_PORT);

    UDPsocket udp_server = SDLNet_UDP_Open(SERVER_PORT);

    if (!udp_server)
    {
        SDL_Log("SDLNet_UDP_Open: %s", SDLNet_GetError());

        return 1;
    }

    UDPpacket *packet = SDLNet_AllocPacket(PACKET_SIZE);

    if (!packet)
    {
        SDL_Log("SDLNet_AllocPacket: %s", SDLNet_GetError());

        return 1;
    }

    bool quit = false;
    while (!quit)
    {
        {
            TCPsocket client = SDLNet_TCP_Accept(tcp_server);

            if (client)
            {
                int client_id = get_client_id();

                if (client_id != -1)
                {
                    SDL_Log("Connected to client %s:%i", SDLNet_ResolveIP(&ip), SDLNet_Read16(&ip.port));

                    tcp_clients[client_id].id = client_id;
                    tcp_clients[client_id].socket = client;

                    SDLNet_TCP_AddSocket(sockets, client);

                    {
                        char send[PACKET_SIZE];
                        sprintf_s(send, sizeof(send), "Hello, Client!");
                        SDLNet_TCP_Send(client, send, strlen(send) + 1);
                    }

                    int num_clients = 0;

                    for (int i = 0; i < MAX_SOCKETS; i++)
                    {
                        if (tcp_clients[i].id != -1)
                        {
                            num_clients++;

                            if (tcp_clients[i].id != client_id)
                            {
                                char send[PACKET_SIZE];
                                sprintf_s(send, sizeof(send), "A client has connected!");
                                SDLNet_TCP_Send(tcp_clients[i].socket, send, strlen(send) + 1);
                            }
                        }
                    }

                    SDL_Log("There are %d clients connected", num_clients);
                }
                else
                {
                    char send[PACKET_SIZE];
                    sprintf_s(send, sizeof(send), "Server is full!");
                    SDLNet_TCP_Send(client, send, strlen(send) + 1);
                }
            }
        }

        {
            while (SDLNet_CheckSockets(sockets, 0) > 0)
            {
                for (int i = 0; i < MAX_SOCKETS; i++)
                {
                    if (tcp_clients[i].id != -1)
                    {
                        if (SDLNet_SocketReady(tcp_clients[i].socket))
                        {
                            char recv[PACKET_SIZE];
                            int bytes = SDLNet_TCP_Recv(tcp_clients[i].socket, recv, sizeof(recv));
                            if (bytes > 0)
                            {
                                SDL_Log("Recieved %d bytes: %s", bytes, recv);

                                if (strcmp(recv, "Goodbye, Server!") == 0)
                                {
                                    SDL_Log("Disconnected from client %s:%i", SDLNet_ResolveIP(&ip), SDLNet_Read16(&ip.port));

                                    SDLNet_TCP_DelSocket(sockets, tcp_clients[i].socket);
                                    SDLNet_TCP_Close(tcp_clients[i].socket);

                                    tcp_clients[i].id = -1;
                                    tcp_clients[i].socket = NULL;

                                    int num_clients = 0;

                                    for (int j = 0; j < MAX_SOCKETS; j++)
                                    {
                                        if (tcp_clients[j].id != -1)
                                        {
                                            num_clients++;

                                            char send[PACKET_SIZE];
                                            sprintf_s(send, sizeof(send), "A client has disconnected!");
                                            SDLNet_TCP_Send(tcp_clients[j].socket, send, strlen(send) + 1);
                                        }
                                    }

                                    SDL_Log("There are %d clients connected", num_clients);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(udp_server);
    SDLNet_FreeSocketSet(sockets);
    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        if (tcp_clients[i].id != -1)
        {
            SDLNet_TCP_Close(tcp_clients[i].socket);
        }
    }
    SDLNet_TCP_Close(tcp_server);
    SDLNet_Quit();

    SDL_Quit();

    return 0;
}

int get_client_id(void)
{
    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        if (tcp_clients[i].id == -1)
        {
            return i;
        }
    }

    return -1;
}
