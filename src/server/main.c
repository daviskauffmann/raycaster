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
    double x;
    double y;
} Client;

IPaddress server_address;
TCPsocket tcp_socket = NULL;
TCPpacket *tcp_packet = NULL;
SDLNet_SocketSet tcp_sockets = NULL;
UDPsocket udp_socket = NULL;
UDPpacket *udp_packet = NULL;

Client clients[MAX_SOCKETS];

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

    if (SDLNet_ResolveHost(&server_address, NULL, SERVER_PORT) != 0)
    {
        SDL_Log("SDLNet_ResolveHost: %s", SDLNet_GetError());

        return 1;
    }

    tcp_socket = SDLNet_TCP_Open(&server_address);

    if (!tcp_socket)
    {
        SDL_Log("SDLNet_TCP_Open: %s", SDLNet_GetError());

        return 1;
    }

    tcp_packet = SDLNet_TCP_AllocPacket(PACKET_SIZE);

    if (!tcp_packet)
    {
        SDL_Log("SDLNet_TCP_AllocPacket: %s", SDLNet_GetError());

        return 1;
    }

    tcp_sockets = SDLNet_AllocSocketSet(MAX_SOCKETS);

    if (!tcp_sockets)
    {
        SDL_Log("SDLNet_AllocSocketSet: %s", SDLNet_GetError());

        return 1;
    }

    SDL_Log("Listening on port %d", SERVER_PORT);

    udp_socket = SDLNet_UDP_Open(SERVER_PORT);

    if (!udp_socket)
    {
        SDL_Log("SDLNet_UDP_Open: %s", SDLNet_GetError());

        return 1;
    }

    udp_packet = SDLNet_UDP_AllocPacket(PACKET_SIZE);

    if (!udp_packet)
    {
        SDL_Log("SDLNet_UDP_AllocPacket: %s", SDLNet_GetError());

        return 1;
    }

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
                // search for an empty client
                int client_id = -1;

                for (int i = 0; i < MAX_SOCKETS; i++)
                {
                    if (clients[i].id == -1)
                    {
                        client_id = i;

                        break;
                    }
                }

                if (client_id != -1)
                {
                    // initialize the client
                    clients[client_id].id = client_id;
                    clients[client_id].socket = socket;
                    clients[client_id].x = 22.0;
                    clients[client_id].y = 11.5;

                    // add to the socket list
                    SDLNet_TCP_AddSocket(tcp_sockets, clients[client_id].socket);

                    // send the client their info
                    tcp_send(socket, "%d %d %lf %lf", PACKET_ENTER, clients[client_id].id, clients[client_id].x, clients[client_id].y);

                    // logging
                    IPaddress *client_address = SDLNet_TCP_GetPeerAddress(clients[client_id].socket);
                    const char *client_host = SDLNet_ResolveIP(client_address);
                    unsigned short client_port = SDLNet_Read16(&client_address->port);

                    SDL_Log("Connected to client %s:%d", client_host, client_port);

                    // inform other clients
                    int num_clients = 0;

                    for (int i = 0; i < MAX_SOCKETS; i++)
                    {
                        if (clients[i].id != -1)
                        {
                            num_clients++;

                            if (clients[i].id != clients[client_id].id)
                            {
                                tcp_send(clients[i].socket, "%d %d", PACKET_CONNECT, clients[client_id].id);
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
                                // inform other clients
                                int num_clients = 0;

                                for (int j = 0; j < MAX_SOCKETS; j++)
                                {
                                    if (clients[j].id != -1 && clients[j].id != clients[i].id)
                                    {
                                        num_clients++;

                                        tcp_send(clients[j].socket, "%d %d", PACKET_DISCONNECT, clients[i].id);
                                    }
                                }

                                SDL_Log("There are %d clients connected", num_clients);

                                // logging
                                IPaddress *client_address = SDLNet_TCP_GetPeerAddress(clients[i].socket);
                                const char *client_host = SDLNet_ResolveIP(client_address);
                                unsigned short client_port = SDLNet_Read16(&client_address->port);

                                SDL_Log("Disconnected from client %s:%d", client_host, client_port);

                                // close the TCP connection
                                SDLNet_TCP_DelSocket(tcp_sockets, clients[i].socket);
                                SDLNet_TCP_Close(clients[i].socket);

                                // uninitialize the client
                                clients[i].id = -1;
                                clients[i].socket = NULL;
                            }
                            break;
                            default:
                            {
                                SDL_Log("TCP: Unknown packet type");
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
                sscanf_s((const char *)udp_packet->data, "%d %d %lf %lf", &type, &client_id, &pos_x, &pos_y);

                // update the client's position
                clients[client_id].x = pos_x;
                clients[client_id].y = pos_y;

                // send the new position to all clients
            }
            break;
            default:
            {
                SDL_Log("UDP: Unknown packet type");
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
