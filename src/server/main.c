#include <SDL/SDL_net.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../shared/map.h"
#include "../shared/net.h"

#define SDL_FLAGS 0

#define SERVER_PORT 1000

// TODO: handle timeouts on clients to automatically disconnect them
typedef struct
{
    int id;
    TCPsocket socket;
    IPaddress udp_address;
} Client;

IPaddress server_address;
const char *server_host = NULL;
unsigned short server_port;

TCPsocket tcp_socket = NULL;
SDLNet_SocketSet tcp_sockets = NULL;
char tcp_buffer[PACKET_SIZE];

UDPsocket udp_socket = NULL;
UDPpacket *udp_packet = NULL;

Client clients[MAX_CLIENTS];
Player players[MAX_CLIENTS];

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

    server_host = SDLNet_ResolveIP(&server_address);
    server_port = SDLNet_Read16(&server_address.port);

    tcp_socket = SDLNet_TCP_Open(&server_address);

    if (!tcp_socket)
    {
        SDL_Log("SDLNet_TCP_Open: %s", SDLNet_GetError());

        return 1;
    }

    tcp_sockets = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);

    if (!tcp_sockets)
    {
        SDL_Log("SDLNet_AllocSocketSet: %s", SDLNet_GetError());

        return 1;
    }

    SDLNet_TCP_AddSocket(tcp_sockets, tcp_socket);

    SDL_Log("Listening on port %d", SERVER_PORT);

    udp_socket = SDLNet_UDP_Open(SERVER_PORT);

    if (!udp_socket)
    {
        SDL_Log("SDLNet_UDP_Open: %s", SDLNet_GetError());

        return 1;
    }

    udp_packet = SDLNet_AllocPacket(PACKET_SIZE);

    if (!udp_packet)
    {
        SDL_Log("SDLNet_UDP_AllocPacket: %s", SDLNet_GetError());

        return 1;
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].id = -1;
        clients[i].socket = NULL;

        players[i].id = -1;
        players[i].pos_x = 22.0;
        players[i].pos_y = 11.5;
        players[i].dir_x = -1.0;
        players[i].dir_y = 0.0;
        players[i].plane_x = 0.0;
        players[i].plane_y = 1.0;
    }

    bool quit = false;

    while (!quit)
    {
        // handle TCP messages
        if (SDLNet_CheckSockets(tcp_sockets, 0) > 0)
        {
            // check activity on the server
            if (SDLNet_SocketReady(tcp_socket))
            {
                // accept new clients
                TCPsocket socket = SDLNet_TCP_Accept(tcp_socket);

                if (socket)
                {
                    // search for an empty client
                    int client_id = -1;

                    for (int i = 0; i < MAX_CLIENTS; i++)
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

                        players[client_id].id = client_id;

                        // add to the socket list
                        SDLNet_TCP_AddSocket(tcp_sockets, clients[client_id].socket);

                        // send the client their info
                        {
                            ConnectData connect_data = connect_data_create(DATA_CONNECT_OK, clients[client_id].id, players);
                            SDLNet_TCP_Send(socket, &connect_data, sizeof(connect_data));
                        }

                        // inform other clients
                        for (int i = 0; i < MAX_CLIENTS; i++)
                        {
                            if (clients[i].id != -1 && clients[i].id != clients[client_id].id)
                            {
                                PlayerData player_data = player_data_create(DATA_CONNECT_BROADCAST, players[client_id]);
                                SDLNet_TCP_Send(clients[i].socket, &player_data, sizeof(player_data));
                            }
                        }

                        // logging
                        {
                            IPaddress *address = SDLNet_TCP_GetPeerAddress(clients[client_id].socket);
                            const char *host = SDLNet_ResolveIP(address);
                            unsigned short port = SDLNet_Read16(&address->port);

                            SDL_Log("Connected to client %s:%d", host, port);

                            int num_clients = 0;

                            for (int i = 0; i < MAX_CLIENTS; i++)
                            {
                                if (clients[i].id != -1)
                                {
                                    num_clients++;
                                }
                            }

                            SDL_Log("There are %d clients connected", num_clients);
                        }
                    }
                    else
                    {
                        Data data = data_create(DATA_CONNECT_FULL);
                        SDLNet_TCP_Send(socket, &data, sizeof(data));

                        SDL_Log("A client tried to connect, but the server is full");
                    }
                }
            }

            // check activity on each client
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].id != -1)
                {
                    if (SDLNet_SocketReady(clients[i].socket))
                    {
                        if (SDLNet_TCP_Recv(clients[i].socket, tcp_buffer, sizeof(tcp_buffer)) > 0)
                        {
                            switch (((Data *)tcp_buffer)->type)
                            {
                            case DATA_DISCONNECT_REQUEST:
                            {
                                // logging
                                {
                                    IPaddress *address = SDLNet_TCP_GetPeerAddress(clients[i].socket);
                                    const char *host = SDLNet_ResolveIP(address);
                                    unsigned short port = SDLNet_Read16(&address->port);

                                    SDL_Log("Disconnecting from client %s:%d", host, port);

                                    int num_clients = 0;

                                    for (int j = 0; j < MAX_CLIENTS; j++)
                                    {
                                        if (clients[j].id != -1 && clients[j].id != clients[i].id)
                                        {
                                            num_clients++;
                                        }
                                    }

                                    SDL_Log("There are %d clients connected", num_clients);
                                }

                                // inform other clients
                                for (int j = 0; j < MAX_CLIENTS; j++)
                                {
                                    if (clients[j].id != -1 && clients[j].id != clients[i].id)
                                    {
                                        IdData id_data = id_data_create(DATA_DISCONNECT_BROADCAST, clients[i].id);
                                        SDLNet_TCP_Send(clients[j].socket, &id_data, sizeof(id_data));
                                    }
                                }

                                // close the TCP connection
                                SDLNet_TCP_DelSocket(tcp_sockets, clients[i].socket);
                                SDLNet_TCP_Close(clients[i].socket);

                                // uninitialize the client
                                clients[i].id = -1;
                                clients[i].socket = NULL;

                                players[i].id = -1;
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
        while (SDLNet_UDP_Recv(udp_socket, udp_packet) != 0)
        {
            switch (((Data *)udp_packet->data)->type)
            {
            case DATA_UDP_CONNECT_REQUEST:
            {
                int id = ((IdData *)udp_packet->data)->id;

                SDL_Log("Saving UDP info of client %d", id);

                clients[id].udp_address = udp_packet->address;
            }
            break;
            case DATA_MOVEMENT_REQUEST:
            {
                int id = ((PosData *)udp_packet->data)->id;
                double pos_x = ((PosData *)udp_packet->data)->pos_x;
                double pos_y = ((PosData *)udp_packet->data)->pos_y;

                SDL_Log("Updating position of client %d to (%lf, %lf)", id, pos_x, pos_y);

                // TODO: perform validation

                // update the player's position
                players[id].pos_x = pos_x;
                players[id].pos_y = pos_y;

                // inform other clients
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients[i].id != -1)
                    {
                        PosData pos_data = pos_data_create(DATA_DISCONNECT_BROADCAST, id, pos_x, pos_y);
                        udp_packet->address = clients[i].udp_address;
                        udp_packet->data = (Uint8 *)&pos_data;
                        udp_packet->len = sizeof(pos_data);
                        SDLNet_UDP_Send(udp_socket, -1, udp_packet);
                    }
                }
            }
            break;
            default:
            {
                SDL_Log("UDP: Unknown packet type");
            }
            break;
            }
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].id != -1)
        {
            SDLNet_TCP_DelSocket(tcp_sockets, clients[i].socket);
            SDLNet_TCP_Close(clients[i].socket);

            clients[i].id = -1;
            clients[i].socket = NULL;

            players[i].id = -1;
        }
    }

    SDLNet_FreePacket(udp_packet);
    SDLNet_UDP_Close(udp_socket);
    SDLNet_FreeSocketSet(tcp_sockets);
    SDLNet_TCP_Close(tcp_socket);
    SDLNet_Quit();

    SDL_Quit();

    return 0;
}
