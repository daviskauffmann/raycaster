#ifndef PLAYER_H
#define PLAYER_H

#define MAX_PLAYERS 2

typedef struct
{
    int id;
    double pos_x;
    double pos_y;
    double dir_x;
    double dir_y;
    double plane_x;
    double plane_y;
} Player;

extern Player players[MAX_PLAYERS];

#endif
