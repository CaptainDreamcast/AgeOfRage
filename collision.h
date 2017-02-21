#ifndef COLLISION_H
#define COLLISION_H

#include <tari/physics.h>

typedef struct {	
	Acceleration force;
	int strength;
	int id;
} CollisionData;

void loadCollision();
CollisionData makeHittableCollisionData();
CollisionData makePunchCollisionData(int strength, Acceleration force);
int getEnemyCollisionListID();
int getPlayerCollisionListID();
int getPlayerAttackCollisionListID();

void updateCollisionDataID(CollisionData* cData);


#endif
