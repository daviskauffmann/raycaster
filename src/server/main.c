#include <SDL/SDL_net.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../shared/data.h"
#include "../shared/map.h"
#include "../shared/player.h"
#include "../shared/SDL_net_ext.h"

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
TCPpacket *tcp_packet = NULL;

UDPsocket udp_socket = NULL;
UDPpacket *udp_packet = NULL;

SDLNet_SocketSet socket_set = NULL;

Client clients[MAX_PLAYERS];

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

    SDL_Log("Listening on port %d", SERVER_PORT);

    tcp_packet = SDLNet_TCP_AllocPacket(PACKET_SIZE);

    if (!tcp_packet)
    {
        SDL_Log("SDLNet_TCP_AllocPacket: %s", SDLNet_GetError());

        return 1;
    }

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

    socket_set = SDLNet_AllocSocketSet(MAX_PLAYERS + 2);

    if (!socket_set)
    {
        SDL_Log("SDLNet_AllocSocketSet: %s", SDLNet_GetError());

        return 1;
    }

    SDLNet_TCP_AddSocket(socket_set, tcp_socket);
    SDLNet_UDP_AddSocket(socket_set, udp_socket);

    for (int i = 0; i < MAX_PLAYERS; i++)
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
        if (SDLNet_CheckSockets(socket_set, 0) > 0)
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

                    for (int i = 0; i < MAX_PLAYERS; i++)
                    {
                        if (clients[i].id == -1)
                        {
                            client_id = i;

                            break;
                        }
                    }

                    if (client_id != -1)
                    {
                        // get socket info
                        IPaddress *address = SDLNet_TCP_GetPeerAddress(socket);
                        const char *host = SDLNet_ResolveIP(address);
                        unsigned short port = SDLNet_Read16(&address->port);

                        SDL_Log("Connected to client %s:%d", host, port);

                        // initialize the client
                        clients[client_id].id = client_id;
                        clients[client_id].socket = socket;

                        players[client_id].id = client_id;

                        // add to the socket list
                        SDLNet_TCP_AddSocket(socket_set, clients[client_id].socket);

                        // send the client their info
                        {
                            struct state_data state_data = state_data_create(DATA_CONNECT_OK, clients[client_id].id);
                            SDLNet_TCP_SendExt(socket, &state_data, sizeof(state_data));
                        }

                        // inform other clients
                        for (int i = 0; i < MAX_PLAYERS; i++)
                        {
                            if (clients[i].id != -1 && clients[i].id != clients[client_id].id)
                            {
                                struct player_data player_data = player_data_create(DATA_CONNECT_BROADCAST, players[client_id]);
                                SDLNet_TCP_SendExt(clients[i].socket, &player_data, sizeof(player_data));
                            }
                        }

                        // log the current number of clients
                        {
                            int num_clients = 0;

                            for (int i = 0; i < MAX_PLAYERS; i++)
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
                        SDL_Log("A client tried to connect, but the server is full");

                        struct data data = data_create(DATA_CONNECT_FULL);
                        SDLNet_TCP_SendExt(socket, &data, sizeof(data));
                    }
                }
            }

            // check activity on each client
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (clients[i].id != -1)
                {
                    if (SDLNet_SocketReady(clients[i].socket))
                    {
                        if (SDLNet_TCP_RecvExt(clients[i].socket, tcp_packet) == 1)
                        {
                            struct data *data = (struct data *)tcp_packet->data;

                            switch (data->type)
                            {
                            case DATA_DISCONNECT_REQUEST:
                            {
                                // get socket info
                                IPaddress *address = SDLNet_TCP_GetPeerAddress(clients[i].socket);
                                const char *host = SDLNet_ResolveIP(address);
                                unsigned short port = SDLNet_Read16(&address->port);

                                SDL_Log("Disconnecting from client %s:%d", host, port);

                                // inform other clients
                                for (int j = 0; j < MAX_PLAYERS; j++)
                                {
                                    if (clients[j].id != -1 && clients[j].id != clients[i].id)
                                    {
                                        struct id_data id_data = id_data_create(DATA_DISCONNECT_BROADCAST, clients[i].id);
                                        SDLNet_TCP_SendExt(clients[j].socket, &id_data, sizeof(id_data));
                                    }
                                }

                                // close the TCP connection
                                SDLNet_TCP_DelSocket(socket_set, clients[i].socket);
                                SDLNet_TCP_Close(clients[i].socket);

                                // uninitialize the client
                                clients[i].id = -1;
                                clients[i].socket = NULL;

                                players[i].id = -1;

                                // log the current number of clients
                                {
                                    int num_clients = 0;

                                    for (int j = 0; j < MAX_PLAYERS; j++)
                                    {
                                        if (clients[j].id != -1)
                                        {
                                            num_clients++;
                                        }
                                    }

                                    SDL_Log("There are %d clients connected", num_clients);
                                }
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

            // handle UDP messages
            if (SDLNet_SocketReady(udp_socket))
            {
                if (SDLNet_UDP_RecvExt(udp_socket, udp_packet) == 1)
                {
                    struct data *data = (struct data *)udp_packet->data;

                    switch (data->type)
                    {
                    case DATA_UDP_CONNECT_REQUEST:
                    {
                        struct id_data *id_data = (struct id_data *)data;

                        SDL_Log("Saving UDP info of client %d", id_data->id);

                        clients[id_data->id].udp_address = udp_packet->address;
                    }
                    break;
                    case DATA_MOVEMENT_REQUEST:
                    {
                        struct move_data *move_data = (struct move_data *)data;

                        SDL_Log("Changing position of client %d by (%lf, %lf)", move_data->id, move_data->dx, move_data->dy);

                        // update the player's position
                        player_move(&players[move_data->id], move_data->dx, move_data->dy);

                        // inform other clients
                        for (int i = 0; i < MAX_PLAYERS; i++)
                        {
                            if (clients[i].id != -1)
                            {
                                struct pos_data pos_data = pos_data_create(DATA_MOVEMENT_BROADCAST, move_data->id, players[move_data->id].pos_x, players[move_data->id].pos_y);
                                SDLNet_UDP_SendExt(udp_socket, udp_packet, clients[i].udp_address, &pos_data, sizeof(pos_data));
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
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (clients[i].id != -1)
        {
            SDLNet_TCP_DelSocket(socket_set, clients[i].socket);
            SDLNet_TCP_Close(clients[i].socket);

            clients[i].id = -1;
            clients[i].socket = NULL;

            players[i].id = -1;
        }
    }

    SDLNet_UDP_DelSocket(socket_set, udp_socket);
    SDLNet_TCP_DelSocket(socket_set, tcp_socket);
    SDLNet_FreeSocketSet(socket_set);
    SDLNet_UDP_FreePacket(udp_packet);
    SDLNet_UDP_Close(udp_socket);
    SDLNet_TCP_FreePacket(tcp_packet);
    SDLNet_TCP_Close(tcp_socket);
    SDLNet_Quit();

    SDL_Quit();

    return 0;
}
