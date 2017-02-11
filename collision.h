#ifndef COLLISION_H
#define COLLISION_H

#include <tari/physics.h>

typedef struct {	
	Acceleration force;
	int strength;
} CollisionData;

void loadCollision();
CollisionData makeHittableCollisionData();
int getEnemyCollisionListID();


#endif
