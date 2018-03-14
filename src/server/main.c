#include <SDL/SDL_net.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../shared/net.h"
#include "../shared/object.h"
#include "../shared/utils.h"

#define SDL_FLAGS 0

#define SERVER_PORT 1000
#define MAX_SOCKETS 2

typedef struct client_s
{
    int id;
    TCPsocket socket;
} client_t;

int get_new_client(client_t clients[MAX_SOCKETS]);

int main(int argc, char *args[])
{
    // suppress warning C4100
    (void)argc;
    (void)args;

    if (SDL_Init(SDL_FLAGS) != 0)
    {
        SDL_Log("SDL_Init: %s", SDL_GetError());

        return 1;
    }

    if (SDLNet_Init() != 0)
    {
        SDL_Log("SDLNet_Init: %s", SDLNet_GetError());

        return 1;
    }

    IPaddress address;

    if (SDLNet_ResolveHost(&address, NULL, SERVER_PORT) != 0)
    {
        SDL_Log("SDLNet_ResolveHost: %s", SDLNet_GetError());

        return 1;
    }

    TCPsocket tcp_socket = SDLNet_TCP_Open(&address);

    if (!tcp_socket)
    {
        SDL_Log("SDLNet_TCP_Open: %s", SDLNet_GetError());

        return 1;
    }

    TCPpacket *tcp_packet = SDLNet_TCP_AllocPacket(PACKET_SIZE);

    if (!tcp_packet)
    {
        SDL_Log("SDLNet_TCP_AllocPacket: %s", SDLNet_GetError());

        return 1;
    }

    SDLNet_SocketSet tcp_sockets = SDLNet_AllocSocketSet(MAX_SOCKETS);

    if (!tcp_sockets)
    {
        SDL_Log("SDLNet_AllocSocketSet: %s", SDLNet_GetError());

        return 1;
    }

    SDL_Log("Listening on port %d", SERVER_PORT);

    UDPsocket udp_socket = SDLNet_UDP_Open(SERVER_PORT);

    if (!udp_socket)
    {
        SDL_Log("SDLNet_UDP_Open: %s", SDLNet_GetError());

        return 1;
    }

    UDPpacket *udp_packet = SDLNet_UDP_AllocPacket(PACKET_SIZE);

    if (!udp_packet)
    {
        SDL_Log("SDLNet_UDP_AllocPacket: %s", SDLNet_GetError());

        return 1;
    }

    client_t clients[MAX_SOCKETS];

    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        clients[i].id = -1;
        clients[i].socket = NULL;
    }

    bool quit = false;
    while (!quit)
    {
        // accept new clients
        {
            TCPsocket socket = SDLNet_TCP_Accept(tcp_socket);

            if (socket)
            {
                int client_id = get_new_client(clients);

                if (client_id != -1)
                {
                    IPaddress *client_address = SDLNet_TCP_GetPeerAddress(socket);
                    const char *client_host = SDLNet_ResolveIP(client_address);
                    unsigned short client_port = SDLNet_Read16(&client_address->port);

                    SDL_Log("Connected to client %s:%d", client_host, client_port);

                    clients[client_id].id = client_id;
                    clients[client_id].socket = socket;

                    SDLNet_TCP_AddSocket(tcp_sockets, socket);

                    tcp_send(socket, "Hello, Client!");

                    int num_clients = 0;

                    for (int i = 0; i < MAX_SOCKETS; i++)
                    {
                        if (clients[i].id != -1)
                        {
                            num_clients++;

                            if (clients[i].id != client_id)
                            {
                                tcp_send(clients[i].socket, "A client has connected! ID: %d", client_id);
                            }
                        }
                    }

                    SDL_Log("There are %d clients connected", num_clients);
                }
                else
                {
                    SDL_Log("Client tried to connect, but server is full");

                    tcp_send(socket, "Server is full!");
                }
            }
        }

        // handle TCP messages
        if (SDLNet_CheckSockets(tcp_sockets, 0) > 0)
        {
            for (int i = 0; i < MAX_SOCKETS; i++)
            {
                if (clients[i].id != -1)
                {
                    if (SDLNet_SocketReady(clients[i].socket))
                    {
                        if (tcp_recv(clients[i].socket, tcp_packet) > 0)
                        {
                            // TEST: sending large data
                            if (strcmp((const char *)tcp_packet->data, "Requesting map!") == 0)
                            {
                                tcp_send(clients[i].socket, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
                            }
                            // TEST: relaying message to all clients
                            else if (strcmp((const char *)udp_packet->data, "I am shooting!") == 0)
                            {
                                for (int j = 0; j < MAX_SOCKETS; j++)
                                {
                                    if (clients[j].id != -1)
                                    {
                                        tcp_send(clients[j].socket, "Someone is shooting!");
                                    }
                                }
                            }
                            // handle client disconnect
                            else if (strcmp((const char *)tcp_packet->data, "Goodbye, Server!") == 0)
                            {
                                int client_id = clients[i].id;
                                IPaddress *client_address = SDLNet_TCP_GetPeerAddress(clients[i].socket);
                                const char *client_host = SDLNet_ResolveIP(client_address);
                                unsigned short client_port = SDLNet_Read16(&client_address->port);

                                SDL_Log("Disconnected from client %s:%d", client_host, client_port);

                                SDLNet_TCP_DelSocket(tcp_sockets, clients[i].socket);
                                SDLNet_TCP_Close(clients[i].socket);

                                clients[i].id = -1;
                                clients[i].socket = NULL;

                                int num_clients = 0;

                                for (int j = 0; j < MAX_SOCKETS; j++)
                                {
                                    if (clients[j].id != -1)
                                    {
                                        num_clients++;

                                        tcp_send(clients[j].socket, "A client has disconnected! ID: %d", client_id);
                                    }
                                }

                                SDL_Log("There are %d clients connected", num_clients);
                            }
                        }
                    }
                }
            }
        }

        // handle UDP messages
        while (udp_recv(udp_socket, udp_packet) > 0)
        {
        }

        SDL_Delay(100);
    }

    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        if (clients[i].id != -1)
        {
            SDLNet_TCP_DelSocket(tcp_sockets, clients[i].socket);
            SDLNet_TCP_Close(clients[i].socket);

            clients[i].id = -1;
            clients[i].socket = NULL;
        }
    }

    SDLNet_UDP_FreePacket(udp_packet);
    SDLNet_UDP_Close(udp_socket);
    SDLNet_FreeSocketSet(tcp_sockets);
    SDLNet_TCP_FreePacket(tcp_packet);
    SDLNet_TCP_Close(tcp_socket);
    SDLNet_Quit();

    SDL_Quit();

    return 0;
}

int get_new_client(client_t clients[MAX_SOCKETS])
{
    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        if (clients[i].id == -1)
        {
            return i;
        }
    }

    return -1;
}
