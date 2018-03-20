#include "data.h"
#include "map.h"
#include "player.h"

struct data data_create(enum data_type type)
{
    struct data data;

    data.type = type;

    return data;
}

struct id_data id_data_create(enum data_type type, int id)
{
    struct id_data id_data;

    id_data.data = data_create(type);
    id_data.id = id;

    return id_data;
}

struct state_data state_data_create(enum data_type type, int id)
{
    struct state_data state_data;

    state_data.data = data_create(type);
    state_data.id = id;
    for (int i = 0; i < NUM_OBJECTS; i++)
    {
        state_data.objects[i] = objects[i];
    }
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        state_data.players[i] = players[i];
    }

    return state_data;
}

struct player_data player_data_create(enum data_type type, struct player player)
{
    struct player_data player_data;

    player_data.data = data_create(type);
    player_data.player = player;

    return player_data;
}

struct move_data move_data_create(enum data_type type, int id, double dx, double dy)
{
    struct move_data move_data;

    move_data.data = data_create(type);
    move_data.id = id;
    move_data.dx = dx;
    move_data.dy = dy;

    return move_data;
}

struct pos_data pos_data_create(enum data_type type, int id, double pos_x, double pos_y)
{
    struct pos_data pos_data;

    pos_data.data = data_create(type);
    pos_data.id = id;
    pos_data.pos_x = pos_x;
    pos_data.pos_y = pos_y;

    return pos_data;
}
