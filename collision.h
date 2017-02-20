#ifndef COLLISION_H
#define COLLISION_H

#include <tari/physics.h>

typedef struct {	
	Acceleration force;
	int strength;
} CollisionData;

void loadCollision();
CollisionData makeHittableCollisionData();
CollisionData makePunchCollisionData(int strength, Acceleration force);
int getEnemyCollisionListID();
int getPlayerAttackCollisionListID();


#endif
