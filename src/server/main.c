#include <SDL/SDL_net.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../shared/map.h"
#include "../shared/net.h"
#include "../shared/util.h"

#define SDL_FLAGS 0

#define SERVER_PORT 1000
#define MAX_SOCKETS 2

// TODO: handle timeouts on clients to automatically disconnect them
typedef struct
{
    int id;
    TCPsocket socket;
} Client;

int get_new_client(Client clients[MAX_SOCKETS]);

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

    Client clients[MAX_SOCKETS];

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

                    tcp_send(socket, "%d,%d", PACKET_ENTER, client_id);

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

                    tcp_send(socket, "%d", PACKET_FULL);
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
                            int type;
                            sscanf_s((const char *)tcp_packet->data, "%d", &type);

                            switch (type)
                            {
                            case PACKET_DISCONNECT:
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
                            break;
                            default:
                            {
                                SDL_Log("Unknown packet type");
                            }
                            break;
                            }
                        }
                    }
                }
            }
        }

        // handle UDP messages
        while (udp_recv(udp_socket, udp_packet) > 0)
        {
            int type;
            sscanf_s((const char *)udp_packet->data, "%d", &type);

            switch (type)
            {
            case PACKET_MOVEMENT:
            {
                int client_id;
                double pos_x;
                double pos_y;
                sscanf_s((const char *)udp_packet->data, "%d,%d,%lf,%lf", &type, &client_id, &pos_x, &pos_y);

                SDL_Log("%d: %lf, %lf", client_id, pos_x, pos_y);
            }
            break;
            default:
            {
            }
            break;
            }
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

int get_new_client(Client clients[MAX_SOCKETS])
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
