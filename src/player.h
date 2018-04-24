#ifndef PLAYER_H
#define PLAYER_H

struct player
{
    double pos_x;
    double pos_y;
    double dir_x;
    double dir_y;
    double plane_x;
    double plane_y;
};

void player_move(struct player *player, double dx, double dy);
void player_rotate(struct player *player, double angle);

#endif
