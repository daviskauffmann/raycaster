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

    SDLNet_SocketSet sockets = SDLNet_AllocSocketSet(MAX_SOCKETS);

    if (!sockets)
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

    UDPpacket *packet = SDLNet_AllocPacket(PACKET_SIZE);

    if (!packet)
    {
        SDL_Log("SDLNet_AllocPacket: %s", SDLNet_GetError());

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

                    SDLNet_TCP_AddSocket(sockets, socket);

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

        while (SDLNet_CheckSockets(sockets, 0) > 0)
        {
            for (int i = 0; i < MAX_SOCKETS; i++)
            {
                if (clients[i].id != -1)
                {
                    if (SDLNet_SocketReady(clients[i].socket))
                    {
                        response_t response = tcp_recv(clients[i].socket);

                        if (response.len > 0)
                        {
                            if (strcmp(response.data, "Requesting map!") == 0)
                            {
                                tcp_send(clients[i].socket, "Here is the map!");
                            }
                            else if (strcmp(response.data, "Goodbye, Server!") == 0)
                            {
                                int client_id = clients[i].id;
                                IPaddress *client_address = SDLNet_TCP_GetPeerAddress(clients[i].socket);
                                const char *client_host = SDLNet_ResolveIP(client_address);
                                unsigned short client_port = SDLNet_Read16(&client_address->port);

                                SDL_Log("Disconnected from client %s:%d", client_host, client_port);

                                SDLNet_TCP_DelSocket(sockets, clients[i].socket);
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

        if (udp_recv(udp_socket, packet))
        {
        }
    }

    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        if (clients[i].id != -1)
        {
            SDLNet_TCP_Close(clients[i].socket);

            clients[i].id = -1;
            clients[i].socket = NULL;
        }
    }

    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(udp_socket);
    SDLNet_FreeSocketSet(sockets);
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
