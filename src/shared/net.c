#include "net.h"

Data data_create(DataType type)
{
    Data data;

    data.type = type;

    return data;
}

IdData id_data_create(DataType type, int id)
{
    IdData id_data;

    id_data.data = data_create(type);
    id_data.id = id;

    return id_data;
}

ConnectData connect_data_create(DataType type, int id, Player players[MAX_CLIENTS])
{
    ConnectData connect_data;

    connect_data.data = data_create(type);
    connect_data.id = id;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        connect_data.players[i] = players[i];
    }

    return connect_data;
}

PlayerData player_data_create(DataType type, Player player)
{
    PlayerData player_data;

    player_data.data = data_create(type);
    player_data.player = player;

    return player_data;
}

PosData pos_data_create(DataType type, int id, double pos_x, double pos_y)
{
    PosData pos_data;

    pos_data.data = data_create(type);
    pos_data.id = id;
    pos_data.pos_x = pos_x;
    pos_data.pos_y = pos_y;

    return pos_data;
}
