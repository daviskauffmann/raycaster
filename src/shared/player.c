#include <math.h>

#include "map.h"
#include "player.h"

struct player players[MAX_PLAYERS];

void player_move(struct player *player, double dx, double dy)
{
    if (wall_map[(int)(player->pos_x + dx)][(int)(player->pos_y)] == 0)
    {
        player->pos_x += dx;
    }
    if (wall_map[(int)(player->pos_x)][(int)(player->pos_y + dy)] == 0)
    {
        player->pos_y += dy;
    }
}

void player_rotate(struct player *player, double angle)
{
    double rot_x = cos(angle);
    double rot_y = sin(angle);

    // both camera direction and camera plane must be rotated
    double old_dir_x = player->dir_x;
    player->dir_x = player->dir_x * rot_x - player->dir_y * rot_y;
    player->dir_y = old_dir_x * rot_y + player->dir_y * rot_x;

    double old_plane_x = player->plane_x;
    player->plane_x = player->plane_x * rot_x - player->plane_y * rot_y;
    player->plane_y = old_plane_x * rot_y + player->plane_y * rot_x;
}
