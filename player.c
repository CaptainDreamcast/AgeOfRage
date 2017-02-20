#include "player.h"

#include <tari/system.h>
#include <tari/log.h>
#include <tari/animation.h>
#include <tari/texture.h>
#include <tari/script.h>
#include <tari/physicshandler.h>
#include <tari/input.h>
#include <tari/collision.h>
#include <tari/memoryhandler.h>
#include <tari/datastructures.h>
#include <tari/collisionanimation.h>

#include "collision.h"

typedef enum {
	STATE_IDLE, 
	STATE_WALKING,
	STATE_WEAK_PUNCH
} State;

static struct {

	int physicsID;	
	Position* mPosition;
	Velocity* mVelocity;
	Position mCenter;

	Animation idleAnimation;
	TextureData idleTextures[10];


	Animation walkingAnimation;
	TextureData walkingTextures[10];

	Animation weakPunchAnimation;
	TextureData weakPunchTextures[10];

	Animation strongPunchAnimation;
	TextureData strongPunchTextures[10];

	Animation hitAnimation;
	TextureData hitTextures[10];

	CollisionAnimation weakPunchCollisionAnimation;
	CollisionData weakPunchCollisionData;

	int animationID;
	int collisionID;

	State state;
	int direction;
	
	Collider collider;
	CollisionData collisionData;

	int collisionAnimationID;
} gData;



static void playerHitCB(void* tCaller, void* tCollisionData);

static ScriptPosition loadTextureDataAndAnimation(ScriptPosition position, TextureData* textureData, Animation* animation) {	
	char name[100];
	char path[100];

	resetAnimation(animation);
	int v;
	position = getNextScriptInteger(position, &v);
	animation->mFrameAmount = v;
	position = getNextScriptDouble(position, &animation->mDuration);

	position = getNextScriptString(position, name);

	int i;
	for(i = 0; i < animation->mFrameAmount; i++) {
		sprintf(path, "/sprites/%s%d.pkg", name, i);
		textureData[i] = loadTexture(path);
	}

	return position;
}


static ScriptPosition loadSingleCollisionAnimation(void* caller, ScriptPosition position) {
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

static ScriptPosition loader(void* caller, ScriptPosition position) {
	char word[100];	
	position = getNextScriptString(position, word);

	if(!strcmp(word, "START_POSITION")) {
		Position pos;
		position = getNextScriptDouble(position, &pos.x);
		position = getNextScriptDouble(position, &pos.y);
		position = getNextScriptDouble(position, &pos.z);
		gData.physicsID = addToPhysicsHandler(pos);
		PhysicsObject* physics = getPhysicsFromHandler(gData.physicsID);
		
		gData.mPosition = &physics->mPosition;
		gData.mVelocity = &physics->mVelocity;
	} else if(!strcmp(word, "CENTER_X")) {
		position = getNextScriptDouble(position, &gData.mCenter.x);
		gData.mCenter.y = 0;
		gData.mCenter.z = 0;	
		
	} else if(!strcmp(word, "IDLE_ANIMATION")) {
		position = loadTextureDataAndAnimation(position, gData.idleTextures, &gData.idleAnimation);
	} else if(!strcmp(word, "WALKING_ANIMATION")) {
		position = loadTextureDataAndAnimation(position, gData.walkingTextures, &gData.walkingAnimation);
	} else if(!strcmp(word, "PUNCH_ANIMATION_1")) {
	 	position = loadTextureDataAndAnimation(position, gData.weakPunchTextures, &gData.weakPunchAnimation);
	} else if(!strcmp(word, "PUNCH_COLLISION_ANIMATION_1")) {
	 	ScriptRegion collisionRegion = getScriptRegionAtPosition(position);
		gData.weakPunchCollisionAnimation = makeEmptyCollisionAnimation();
		executeOnScriptRegion(collisionRegion, loadSingleCollisionAnimation, &gData.weakPunchCollisionAnimation);
		gData.weakPunchCollisionAnimation.mAnimation = gData.weakPunchAnimation;
		position = getPositionAfterScriptRegion(position.mRegion, collisionRegion);
	}else if(!strcmp(word, "PUNCH_ANIMATION_2")) {
	 	position = loadTextureDataAndAnimation(position, gData.strongPunchTextures, &gData.strongPunchAnimation);
	} else if(!strcmp(word, "HIT_ANIMATION")) {
	 	position = loadTextureDataAndAnimation(position, gData.hitTextures, &gData.hitAnimation);
	} else if(!strcmp(word, "COLLISION_DATA")) {
		CollisionRect rect;
	 	position = getNextScriptDouble(position, &rect.mTopLeft.x);
		position = getNextScriptDouble(position, &rect.mTopLeft.y);
		position = getNextScriptDouble(position, &rect.mTopLeft.z);
		position = getNextScriptDouble(position, &rect.mBottomRight.x);
		position = getNextScriptDouble(position, &rect.mBottomRight.y);
		position = getNextScriptDouble(position, &rect.mBottomRight.z);
		gData.collider = makeColliderFromRect(rect);
	} else {
		logError("Unknown token.");
		logErrorString(word);
		abortSystem();
	}

	return position;
}

void loadPlayer() {
	Script script = loadScript("/scripts/player.txt");
	ScriptRegion r = getScriptRegion(script, "LOAD");
	executeOnScriptRegion(r, loader, NULL);
	gData.state = STATE_IDLE;
	gData.direction = 1;
	gData.collisionData = makeHittableCollisionData();
	gData.collisionID = addColliderToCollisionHandler(getPlayerAttackCollisionListID(), gData.mPosition, gData.collider, playerHitCB, NULL, &gData.collisionData);
	gData.collisionAnimationID = -1;

	gData.weakPunchCollisionData = makePunchCollisionData(50, makePosition(0,0,0));	

	gData.animationID = playAnimationLoop(makePosition(0,0,0), gData.idleTextures, gData.idleAnimation, makeRectangleFromTexture(gData.idleTextures[0]));
	setAnimationBasePositionReference(gData.animationID, gData.mPosition);
	setAnimationCenter(gData.animationID, gData.mCenter);

	setHandledPhysicsMaxVelocity(gData.physicsID, 1.5);
	setHandledPhysicsDragCoefficient(gData.physicsID, makePosition(0.2, 0.2, 0));
}


static void playerHitCB(void* tCaller, void* tCollisionData) {
	(void) tCaller;
	(void) tCollisionData;
}

static void invert() {
	gData.direction *= -1;
	inverseAnimationVertical(gData.animationID);
	if(gData.collisionAnimationID != -1) {
		invertCollisionAnimationVertical(gData.collisionAnimationID);
	}
}

static void checkInverted() {
	if(gData.direction == 1 && gData.mVelocity->x < 0) invert();
	if(gData.direction == -1 && gData.mVelocity->x > 0) invert();
}

static void setWalking() {
	if(gData.state == STATE_WALKING) return;
	gData.state = STATE_WALKING;
	changeAnimation(gData.animationID, gData.walkingTextures, gData.walkingAnimation, makeRectangleFromTexture(gData.walkingTextures[0]));
}

static void setIdle() {
	if(gData.state == STATE_IDLE) return;
	gData.state = STATE_IDLE;
	*gData.mVelocity = makePosition(0,0,0);
	changeAnimation(gData.animationID, gData.idleTextures, gData.idleAnimation, makeRectangleFromTexture(gData.idleTextures[0]));
}



static void checkMovement() {
	if(gData.state != STATE_IDLE && gData.state != STATE_WALKING) return;
	
	if(hasPressedLeft()) {
		addAccelerationToHandledPhysics(gData.physicsID, makeAcceleration(-1, 0, 0));
		setWalking();
	}

	if(hasPressedRight()) {
		addAccelerationToHandledPhysics(gData.physicsID, makeAcceleration(1, 0, 0));
		setWalking();
	}

	if(hasPressedUp()) {
		addAccelerationToHandledPhysics(gData.physicsID, makeAcceleration(0, -1, 0));
		setWalking();
	}

	if(hasPressedDown()) {
		addAccelerationToHandledPhysics(gData.physicsID, makeAcceleration(0, 1, 0));
		setWalking();
	}

	if(vecLength(*gData.mVelocity) < 0.1) {
		setIdle();
	}

}

static void punchHitSomething(void* tCaller, void* tCollisionData) {
	(void) tCaller;
	(void) tCollisionData;
}

static void weakPunchFinished(void* caller) {
	removeAnimationCB(gData.animationID);
	gData.collisionAnimationID = -1;
	setIdle();
}

static void setWeakPunch() {
	gData.state = STATE_WEAK_PUNCH;
	changeAnimation(gData.animationID, gData.weakPunchTextures, gData.weakPunchAnimation, makeRectangleFromTexture(gData.weakPunchTextures[0]));
	setAnimationCB(gData.animationID, weakPunchFinished);
	gData.collisionAnimationID = addHandledCollisionAnimation(getPlayerAttackCollisionListID(), gData.mPosition, gData.weakPunchCollisionAnimation, punchHitSomething, NULL, &gData.weakPunchCollisionData);
	setCollisionAnimationCenter(gData.collisionAnimationID, gData.mCenter);
	if(gData.direction == -1) {
		invertCollisionAnimationVertical(gData.collisionAnimationID);
	}
}

static void checkWeakPunch()  {
	if(gData.state != STATE_IDLE && gData.state != STATE_WALKING) return;

	if(hasPressedXFlank()) {
		setWeakPunch();
	}
}

static void checkPunch() {
	checkWeakPunch();	


}

void updatePlayer() {
	checkInverted();
	checkMovement();
	checkPunch();
}



Position getPlayerPosition() {
	return *gData.mPosition;
}

void setPlayerScreenPositionReference(Position* p) {
	setAnimationScreenPositionReference(gData.animationID, p);
}
