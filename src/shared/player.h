#ifndef PLAYER_H
#define PLAYER_H

#define MAX_PLAYERS 2

struct player
{
    int id;
    double pos_x;
    double pos_y;
    double dir_x;
    double dir_y;
    double plane_x;
    double plane_y;
};

extern struct player players[MAX_PLAYERS];

void player_move(struct player *player, double dx, double dy);
void player_rotate(struct player *player, double angle);

#endif
