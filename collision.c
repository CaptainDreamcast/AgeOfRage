#include "collision.h"

#include <stdio.h>

#include <tari/collisionhandler.h>

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

int getPlayerCollisionListID() {
 	return gData.playerCollisionList;
}

int getPlayerAttackCollisionListID() {
	return gData.playerAttackCollisionList;
}

void updateCollisionDataID(CollisionData* cData) {
	cData->id = gData.gIDs++;
}
