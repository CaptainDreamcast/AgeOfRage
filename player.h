#ifndef PLAYER_H
#define PLAYER_H

#include <tari/geometry.h>

void loadPlayer();
void addPlayerShadow();
void updatePlayer();
void freezePlayer();
void unfreezePlayer();

Position getPlayerPosition();
void setPlayerScreenPositionReference(Position* p);
int getPlayerHealth();

#endif
