#ifndef COLLISION_H
#define COLLISION_H

#include <tari/physics.h>
#include <tari/script.h>
#include <tari/collisionanimation.h>

typedef struct {	
	Acceleration force;
	int strength;
	int id;
} CollisionData;

void loadCollision();
CollisionData makeHittableCollisionData();
CollisionData makePunchCollisionData(int strength, Acceleration force);
int getEnemyCollisionListID();
int getEnemyAttackCollisionListID();
int getPlayerCollisionListID();
int getPlayerAttackCollisionListID();
void constraintIntoLevel(Position* p, Position* screenPositionReference);

void updateCollisionDataID(CollisionData* cData);
void adjustZ(Position* p);

ScriptPosition loadSingleCollisionAnimation(void* caller, ScriptPosition position);
ScriptPosition loadPunchCollisionAnimation(ScriptPosition position, CollisionAnimation* collisionAnimation, Animation animation);

#endif
