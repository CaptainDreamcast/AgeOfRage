#ifndef PLAYER_H
#define PLAYER_H

#include <tari/geometry.h>

void loadPlayer();
void updatePlayer();

Position getPlayerPosition();
void setPlayerScreenPositionReference(Position* p);

#endif
