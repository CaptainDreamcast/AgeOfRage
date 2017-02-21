#include "enemies.h"

#include <tari/datastructures.h>
#include <tari/animation.h>
#include <tari/texture.h>
#include <tari/collision.h>
#include <tari/script.h>
#include <tari/memoryhandler.h>
#include <tari/physicshandler.h>
#include <tari/collisionhandler.h>
#include <tari/system.h>

#include <tari/log.h>


#include "collision.h"

typedef enum {

	STATE_IDLE,
	STATE_WALKING,
	STATE_HIT,
	STATE_DEATH

} EnemyState;

typedef struct {
	Animation idleAnimation;
	TextureData idleTextures[10];

	Animation walkingAnimation;
	TextureData walkingTextures[10];

	Animation deathAnimation;
	TextureData deathTextures[10];

	Animation hitAnimation;
	TextureData hitTextures[10];

	int health;
	Collider col;

	double maxVelocity;
	Vector3D dragCoefficient;
	
	Position center;


} EnemyType;

typedef struct {

	int id;

	int physicsID;
	int collisionID;
	int animationID;
	int health;
	int type;
	int getHitFromID;
	CollisionData collisionData;

	Position target;
	Position* position;

	EnemyState state;

} ActiveEnemy;

static struct {
	Vector enemyTypes;
	List activeEnemies;
	Position* screenPositionReference;
} gData;

static ScriptPosition loadSingleEnemyTypeAnimation(ScriptPosition pos, Animation* animation, TextureData* textureData) {
	*animation = createEmptyAnimation();
	int v;	
	pos = getNextScriptInteger(pos, &v);
	animation->mFrameAmount = v;
	pos = getNextScriptInteger(pos, &v);
	animation->mDuration = v;
	
	int i;
	for(i = 0; i < animation->mFrameAmount; i++) {
		char file[100];
		char path[100];
		pos = getNextScriptString(pos, file);
		sprintf(path, "/sprites/%s", file);
		textureData[i] = loadTexture(path);
	}

	return pos;
}

static ScriptPosition loadSingleEnemyType(void* caller, ScriptPosition pos) {
	char word[100];
	EnemyType* enemyType = caller;

	pos = getNextScriptString(pos, word);
	if(!strcmp(word, "IDLE_ANIMATION")) {
		pos = loadSingleEnemyTypeAnimation(pos, &enemyType->idleAnimation, enemyType->idleTextures);
	} else if(!strcmp(word, "DEATH_ANIMATION")) {
		pos = loadSingleEnemyTypeAnimation(pos, &enemyType->deathAnimation, enemyType->deathTextures);
	} else if(!strcmp(word, "WALKING_ANIMATION")) {
		pos = loadSingleEnemyTypeAnimation(pos, &enemyType->walkingAnimation, enemyType->walkingTextures);
	} else if(!strcmp(word, "HIT_ANIMATION")) {
		pos = loadSingleEnemyTypeAnimation(pos, &enemyType->hitAnimation, enemyType->hitTextures);
	} else if(!strcmp(word, "HEALTH")) {
		pos = getNextScriptInteger(pos, &enemyType->health);
	} else if(!strcmp(word, "COLLISION")) {
		CollisionRect rect;
		pos = getNextScriptDouble(pos, &rect.mTopLeft.x);
		pos = getNextScriptDouble(pos, &rect.mTopLeft.y);
		pos = getNextScriptDouble(pos, &rect.mTopLeft.z);
		pos = getNextScriptDouble(pos, &rect.mBottomRight.x);
		pos = getNextScriptDouble(pos, &rect.mBottomRight.y);
		pos = getNextScriptDouble(pos, &rect.mBottomRight.z);
		enemyType->col = makeColliderFromRect(rect);
	} else if(!strcmp(word, "MAX_VELOCITY")) {
		pos = getNextScriptDouble(pos, &enemyType->maxVelocity);
	} else if(!strcmp(word, "DRAG_COEFFICIENT")) {
		pos = getNextScriptDouble(pos, &enemyType->dragCoefficient.x);
		pos = getNextScriptDouble(pos, &enemyType->dragCoefficient.y);
		enemyType->dragCoefficient.z = 0;
	} else if(!strcmp(word, "CENTER_X")) {
		pos = getNextScriptDouble(pos, &enemyType->center.x);
		enemyType->center.y = 0;
		enemyType->center.z = 0;
	}else {
		logError("Unrecognized token");
		logErrorString(word);
		abortSystem();
	}

	return pos;
}

static ScriptPosition loader(void* caller, ScriptPosition pos) {
	

	char word[100];
	pos = getNextScriptString(pos, word);

	if(!strcmp(word, "ENEMY_TYPE")) {
		ScriptRegion enemyRegion = getScriptRegionAtPosition(pos);
		EnemyType* enemyType = allocMemory(sizeof(EnemyType));
		executeOnScriptRegion(enemyRegion, loadSingleEnemyType, enemyType);
		vector_push_back_owned(&gData.enemyTypes, enemyType);
		pos = getPositionAfterScriptRegion(pos.mRegion, enemyRegion);
	} else {
		logError("Unrecognized token");
		logErrorString(word);
		abortSystem();
	}


	return pos;
}

static void loadEnemyTypes() {
	Script s = loadScript("/scripts/enemies.txt");
	ScriptRegion r = getScriptRegion(s, "LOAD");
	executeOnScriptRegion(r, loader, NULL);
}


void loadEnemies() {
	gData.enemyTypes = new_vector();
	gData.activeEnemies = new_list();

	loadEnemyTypes();
}

static void checkRandomWalk(ActiveEnemy* enemy) {
	if(enemy->state == STATE_IDLE) {
		checkStartRandomWalk(enemy);
	} else if(enemy->state == STATE_WALKING) {
		checkWalking(enemy);
	}

}

static void checkPunch() {
	if(canHitPlayer()) hit();
}

static void updateSingleEnemy(void* caller, void* data) {
	(void) caller;
	ActiveEnemy* enemy = data;
	checkRandomWalk(enemy);
	checkPunch(enemy);
}

void updateEnemies() {
	list_map(&gData.activeEnemies, updateSingleEnemy, NULL);
}



int getActiveEnemyAmount() {
	return list_size(&gData.activeEnemies);
}


static void removeActiveEnemy(ActiveEnemy* enemy) {
	
	removeHandledAnimation(enemy->animationID);
	removeFromCollisionHandler(getEnemyCollisionListID(), enemy->collisionID);
	removeFromPhysicsHandler(enemy->physicsID);
	list_remove(&gData.activeEnemies, enemy->id);
}

static void dyingOver(void* tCaller) {
	ActiveEnemy* enemy = tCaller;
	removeActiveEnemy(enemy);
}

static void die(ActiveEnemy* enemy) {
	if(enemy->state == STATE_DEATH) return;
	
	EnemyType* enemyType = vector_get(&gData.enemyTypes, enemy->type);

	changeAnimation(enemy->animationID, enemyType->deathTextures, enemyType->deathAnimation, makeRectangleFromTexture(enemyType->deathTextures[0]));
	setAnimationCB(enemy->animationID, dyingOver, enemy);
	enemy->state = STATE_DEATH;
}

static void setIdle(ActiveEnemy* enemy) {
	EnemyType* enemyType = vector_get(&gData.enemyTypes, enemy->type);

	changeAnimation(enemy->animationID, enemyType->idleTextures, enemyType->idleAnimation, makeRectangleFromTexture(enemyType->idleTextures[0]));
	removeAnimationCB(enemy->animationID);
	enemy->state = STATE_IDLE;
}

static void gettingHitOver(void* tCaller) {
	ActiveEnemy* enemy = tCaller;
	setIdle(enemy);
}

static void getHit(ActiveEnemy* enemy) {
	EnemyType* enemyType = vector_get(&gData.enemyTypes, enemy->type);

	changeAnimation(enemy->animationID, enemyType->hitTextures, enemyType->hitAnimation, makeRectangleFromTexture(enemyType->hitTextures[0]));
	setAnimationCB(enemy->animationID, gettingHitOver, enemy);
	enemy->state = STATE_HIT;
}

static void enemyHitCB(void* tCaller, void* tCollisionData) {
	ActiveEnemy* enemy = tCaller;
	CollisionData* cData = tCollisionData;

	if(enemy->getHitFromID == cData->id) return;

	enemy->getHitFromID = cData->id;
	enemy->health -= cData->strength;

	if(enemy->health <= 0) {
		die(enemy);
		return;
	}

	addAccelerationToHandledPhysics(enemy->physicsID, cData->force);

	getHit(enemy);

}


void spawnEnemy(int type, Position pos) {
	EnemyType* enemyType = vector_get(&gData.enemyTypes, type);

	ActiveEnemy* enemy = allocMemory(sizeof(ActiveEnemy));

	enemy->health = enemyType->health;
	enemy->physicsID = addToPhysicsHandler(pos);
	enemy->type = type;
	enemy->getHitFromID = -1;
	
	setHandledPhysicsMaxVelocity(enemy->physicsID, enemyType->maxVelocity);
	setHandledPhysicsDragCoefficient(enemy->physicsID, enemyType->dragCoefficient);

	PhysicsObject* physics = getPhysicsFromHandler(enemy->physicsID);
	enemy->position = &physics->mPosition;
	enemy->target = *enemy->position;

	enemy->collisionData = makeHittableCollisionData();
	enemy->collisionID = addColliderToCollisionHandler(getEnemyCollisionListID(), &physics->mPosition, enemyType->col, enemyHitCB, enemy, &enemy->collisionData);

	enemy->state = STATE_IDLE;
	enemy->animationID = playAnimationLoop(makePosition(0,0,0), enemyType->idleTextures, enemyType->idleAnimation, makeRectangleFromTexture(enemyType->idleTextures[0]));
	setAnimationBasePositionReference(enemy->animationID, &physics->mPosition);
	setAnimationScreenPositionReference(enemy->animationID, gData.screenPositionReference);
	setAnimationCenter(enemy->animationID, enemyType->center);

	enemy->id = list_push_front_owned(&gData.activeEnemies, enemy);
}

void setEnemiesScreenPositionReference(Position* p) {
	gData.screenPositionReference = p;
}

