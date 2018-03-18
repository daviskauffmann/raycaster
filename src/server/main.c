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

Client clients[MAX_SOCKETS];
Player players[MAX_SOCKETS];

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

    udp_packet = SDLNet_AllocPacket(PACKET_SIZE);

    if (!udp_packet)
    {
        SDL_Log("SDLNet_UDP_AllocPacket: %s", SDLNet_GetError());

        return 1;
    }

    for (int i = 0; i < MAX_SOCKETS; i++)
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

                    players[client_id].id = client_id;

                    // add to the socket list
                    SDLNet_TCP_AddSocket(tcp_sockets, clients[client_id].socket);

                    // get info about the socket
                    IPaddress *address = SDLNet_TCP_GetPeerAddress(clients[client_id].socket);
                    const char *host = SDLNet_ResolveIP(address);
                    unsigned short port = SDLNet_Read16(&address->port);

                    SDL_Log("Connected to client %s:%d", host, port);

                    // send the client their info
                    {
                        ConnectData connect_data;
                        connect_data.data.type = DATA_CONNECT_OK;
                        connect_data.id = clients[client_id].id;
                        for (int i = 0; i < MAX_SOCKETS; i++)
                        {
                            connect_data.players[i] = players[i];
                        }
                        SDLNet_TCP_Send(socket, &connect_data, sizeof(connect_data));
                    }

                    int num_clients = 0;

                    // inform other clients
                    for (int i = 0; i < MAX_SOCKETS; i++)
                    {
                        if (clients[i].id != -1)
                        {
                            num_clients++;

                            if (clients[i].id != clients[client_id].id)
                            {
                                PlayerData player_data;
                                player_data.data.type = DATA_CONNECT_BROADCAST;
                                player_data.player = players[client_id];
                                SDLNet_TCP_Send(clients[i].socket, &player_data, sizeof(player_data));
                            }
                        }
                    }

                    SDL_Log("There are %d clients connected", num_clients);
                }
                else
                {
                    SDL_Log("A client tried to connect, but the server is full");

                    Data data;
                    data.type = DATA_CONNECT_FULL;
                    SDLNet_TCP_Send(socket, &data, sizeof(data));
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
                        char buffer[PACKET_SIZE];

                        if (SDLNet_TCP_Recv(clients[i].socket, buffer, sizeof(buffer)) > 0)
                        {
                            Data *data = (Data *)buffer;

                            switch (data->type)
                            {
                            case DATA_DISCONNECT_REQUEST:
                            {
                                // get info about the socket
                                IPaddress *address = SDLNet_TCP_GetPeerAddress(clients[i].socket);
                                const char *host = SDLNet_ResolveIP(address);
                                unsigned short port = SDLNet_Read16(&address->port);

                                SDL_Log("Disconnecting from client %s:%d", host, port);

                                int num_clients = 0;

                                // inform other clients
                                for (int j = 0; j < MAX_SOCKETS; j++)
                                {
                                    if (clients[j].id != -1 && clients[j].id != clients[i].id)
                                    {
                                        num_clients++;

                                        IdData id_data;
                                        id_data.data.type = DATA_DISCONNECT_BROADCAST;
                                        id_data.id = clients[i].id;
                                        SDLNet_TCP_Send(clients[j].socket, &id_data, sizeof(id_data));
                                    }
                                }

                                SDL_Log("There are %d clients connected", num_clients);

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
            Data *data = (Data *)udp_packet->data;

            switch (data->type)
            {
            case DATA_UDP_CONNECT_REQUEST:
            {
                IdData *id_data = (IdData *)data;

                int id = id_data->id;

                SDL_Log("Saving UDP info of client %d", id);

                clients[id].udp_address = udp_packet->address;
            }
            break;
            case DATA_MOVEMENT_REQUEST:
            {
                PosData *pos_data = (PosData *)data;

                int id = pos_data->id;
                double pos_x = pos_data->pos_x;
                double pos_y = pos_data->pos_y;

                SDL_Log("Updating position of client %d to (%lf, %lf)", id, pos_x, pos_y);

                // TODO: perform validation

                // update the player's position
                players[id].pos_x = pos_x;
                players[id].pos_y = pos_y;

                // inform other clients
                for (int i = 0; i < MAX_SOCKETS; i++)
                {
                    if (clients[i].id != -1)
                    {
                        PosData pos_data2;
                        pos_data2.data.type = DATA_DISCONNECT_BROADCAST;
                        pos_data2.id = id;
                        pos_data2.pos_x = pos_x;
                        pos_data2.pos_y = pos_y;
                        udp_packet->address = clients[i].udp_address;
                        udp_packet->data = (Uint8 *)&pos_data2;
                        udp_packet->len = sizeof(pos_data2);
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

    for (int i = 0; i < MAX_SOCKETS; i++)
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
