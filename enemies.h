#ifndef ENEMIES_H
#define ENEMIES_H

#include <tari/geometry.h>

void loadEnemies();
void updateEnemies();
void freezeEnemies();
void unfreezeEnemies();

int getActiveEnemyAmount();
void spawnEnemy(int type, Position pos);
void setEnemiesScreenPositionReference(Position* p);


#endif
