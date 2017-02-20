#include "collision.h"

#include <stdio.h>

#include <tari/collisionhandler.h>

static struct {

	int enemyCollisionList;
	int enemyAttackCollisionList;
	int playerCollisionList;
	int playerAttackCollisionList;

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
	return ret;
}

CollisionData makePunchCollisionData(int strength, Acceleration force) {
	CollisionData ret;
	ret.force = force;
	ret.strength = strength;
	return ret;
}

int getEnemyCollisionListID() {
	return gData.enemyCollisionList;
}

int getPlayerAttackCollisionListID() {
	return gData.playerAttackCollisionList;
}
