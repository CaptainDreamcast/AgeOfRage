#include "collision.h"

#include <stdio.h>
#include <stdlib.h>

#include <tari/collisionhandler.h>
#include <tari/math.h>
#include <tari/memoryhandler.h>

static struct {

	int enemyCollisionList;
	int enemyAttackCollisionList;
	int playerCollisionList;
	int playerAttackCollisionList;
	int gIDs;

} gData;

void loadCollision() {
	gData.enemyCollisionList = addCollisionListToHandler();
	gData.playerCollisionList = addCollisionListToHandler();
	gData.enemyAttackCollisionList = addCollisionListToHandler();
	gData.playerAttackCollisionList = addCollisionListToHandler();

	addCollisionHandlerCheck(gData.enemyCollisionList, gData.playerAttackCollisionList);
	addCollisionHandlerCheck(gData.playerCollisionList, gData.enemyAttackCollisionList);
}

CollisionData makeHittableCollisionData() {
	CollisionData ret;
	ret.force = makePosition(0,0,0);
	ret.strength = 0;
	ret.id = gData.gIDs++;
	return ret;
}

CollisionData makePunchCollisionData(int strength, Acceleration force) {
	CollisionData ret;
	ret.force = force;
	ret.strength = strength;
	ret.id = gData.gIDs++;
	return ret;
}

int getEnemyCollisionListID() {
	return gData.enemyCollisionList;
}

int getEnemyAttackCollisionListID() {
	return gData.enemyAttackCollisionList;
}

int getPlayerCollisionListID() {
 	return gData.playerCollisionList;
}

int getPlayerAttackCollisionListID() {
	return gData.playerAttackCollisionList;
}

void updateCollisionDataID(CollisionData* cData) {
	cData->id = gData.gIDs++;
}

void constraintIntoLevel(Position* p, Position* screenPositionReference) {
	*p = vecAdd(*p, vecScale(*screenPositionReference, -1));
	p->x = max(p->x, -55);
	p->x = min(p->x, 320-64);
	p->y = max(p->y, 20);
	p->y = min(p->y, 240-128);
	*p = vecAdd(*p, *screenPositionReference);
}

void adjustZ(Position* p) {
	double t = getLinearInterpolationFactor(0, 240, p->y);
	p->z = interpolateLinear(3, 4, t);
}

ScriptPosition loadSingleCollisionAnimation(void* caller, ScriptPosition position) {
	CollisionAnimation* cAnimation = caller;

	Position topLeft;
	Position bottomRight;
	position = getNextScriptDouble(position, &topLeft.x);
	position = getNextScriptDouble(position, &topLeft.y);
	position = getNextScriptDouble(position, &topLeft.z);
	position = getNextScriptDouble(position, &bottomRight.x);
	position = getNextScriptDouble(position, &bottomRight.y);
	position = getNextScriptDouble(position, &bottomRight.z);

	CollisionRect rect = makeCollisionRect(topLeft, bottomRight);
	Collider* col = allocMemory(sizeof(Collider));
	*col = makeColliderFromRect(rect);
	vector_push_back_owned(&cAnimation->mFrames, col);

	return position;
}

ScriptPosition loadPunchCollisionAnimation(ScriptPosition position, CollisionAnimation* collisionAnimation, Animation animation) {
	ScriptRegion collisionRegion = getScriptRegionAtPosition(position);
	*collisionAnimation = makeEmptyCollisionAnimation();
	executeOnScriptRegion(collisionRegion, loadSingleCollisionAnimation, collisionAnimation);
	collisionAnimation->mAnimation = animation;
	position = getPositionAfterScriptRegion(position.mRegion, collisionRegion);	
	return position;
}
