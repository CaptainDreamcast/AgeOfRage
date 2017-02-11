#include "enemies.h"

#include <tari/datastructures.h>
#include <tari/animation.h>
#include <tari/texture.h>
#include <tari/collision.h>
#include <tari/script.h>
#include <tari/memoryhandler.h>
#include <tari/physicshandler.h>
#include <tari/collisionhandler.h>

#include <tari/log.h>


#include "collision.h"

typedef struct {
	Animation idleAnimation;
	TextureData idleTextures[10];

	Animation walkingAnimation;
	TextureData walkingTextures[10];

	Animation deathAnimation;
	TextureData deathTextures[10];

	int health;
	Collider col;

} EnemyType;

typedef struct {

	int physicsID;
	int collisionID;
	int health;
	CollisionData collisionData;

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
	} else {
		logError("Unrecognized token");
		logErrorString(word);
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
		pos = getPositionAfterScriptRegion(enemyRegion);
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

void updateEnemies() {
	// via callbacks?
}



int getActiveEnemyAmount() {
	return list_size(&gData.activeEnemies);
}

static void enemyHitCB(void* tCaller, void* tCollisionData) {
	(void) tCaller;
	(void) tCollisionData;
}


void spawnEnemy(int type, Position pos) {
	EnemyType* enemyType = vector_get(&gData.enemyTypes, type);

	ActiveEnemy* enemy = allocMemory(sizeof(ActiveEnemy));

	enemy->health = enemyType->health;
	enemy->physicsID = addToPhysicsHandler(pos);
	PhysicsObject* physics = getPhysicsFromHandler(enemy->physicsID);
	enemy->collisionData = makeHittableCollisionData();
	enemy->collisionID = addColliderToCollisionHandler(getEnemyCollisionListID(), &physics->mPosition, enemyType->col, enemyHitCB, enemy, &enemy->collisionData);
	list_push_front_owned(&gData.activeEnemies, enemy);
}

void setEnemiesScreenPositionReference(Position* p) {
	gData.screenPositionReference = p;
}

