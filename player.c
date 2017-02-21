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
#include <tari/timer.h>

#include "collision.h"

typedef enum {
	STATE_IDLE, 
	STATE_WALKING,
	STATE_WEAK_PUNCH,
	STATE_STRONG_PUNCH,
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

	CollisionAnimation strongPunchCollisionAnimation;
	CollisionData strongPunchCollisionData;

	int animationID;
	int collisionID;

	State state;
	int direction;
	
	int comboState;

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

static ScriptPosition loadPunchCollisionAnimation(ScriptPosition position, CollisionAnimation* collisionAnimation, Animation animation) {
	ScriptRegion collisionRegion = getScriptRegionAtPosition(position);
	*collisionAnimation = makeEmptyCollisionAnimation();
	executeOnScriptRegion(collisionRegion, loadSingleCollisionAnimation, collisionAnimation);
	collisionAnimation->mAnimation = animation;
	position = getPositionAfterScriptRegion(position.mRegion, collisionRegion);	
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
		position = loadPunchCollisionAnimation(position, &gData.weakPunchCollisionAnimation, gData.weakPunchAnimation);
	} else if(!strcmp(word, "PUNCH_ANIMATION_2")) {
	 	position = loadTextureDataAndAnimation(position, gData.strongPunchTextures, &gData.strongPunchAnimation);
	} else if(!strcmp(word, "PUNCH_COLLISION_ANIMATION_2")) {
		position = loadPunchCollisionAnimation(position, &gData.strongPunchCollisionAnimation, gData.strongPunchAnimation);
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
	gData.collisionID = addColliderToCollisionHandler(getPlayerCollisionListID(), gData.mPosition, gData.collider, playerHitCB, NULL, &gData.collisionData);
	gData.collisionAnimationID = -1;
	gData.comboState = 0;

	gData.weakPunchCollisionData = makePunchCollisionData(50, makePosition(0,0,0));	
	gData.strongPunchCollisionData = makePunchCollisionData(100, makePosition(0,0,0));	
	

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


static void resetComboState(void* caller) {
	(void) caller;
	gData.comboState = 0;
}

static void punchHitSomething(void* tCaller, void* tCollisionData) {
	(void) tCaller;
	(void) tCollisionData;

	if(gData.state == STATE_WEAK_PUNCH && !gData.comboState) {
		gData.comboState = 1;
		addTimerCB(30, resetComboState, NULL);

	}
}

static void punchFinished(void* caller) {
	removeAnimationCB(gData.animationID);
	gData.collisionAnimationID = -1;

	setIdle();
}

static void setPunch(State state, TextureData* textures, Animation animation, CollisionAnimation collisionAnimation, CollisionData* collisionData) {
	gData.state = state;
	changeAnimation(gData.animationID, textures, animation, makeRectangleFromTexture(textures[0]));
	setAnimationCB(gData.animationID, punchFinished, NULL);
	updateCollisionDataID(collisionData);
	gData.collisionAnimationID = addHandledCollisionAnimation(getPlayerAttackCollisionListID(), gData.mPosition, collisionAnimation, punchHitSomething, NULL, collisionData);
	setCollisionAnimationCenter(gData.collisionAnimationID, gData.mCenter);
	if(gData.direction == -1) {
		invertCollisionAnimationVertical(gData.collisionAnimationID);
	}
}

static void setStrongPunch() {
	setPunch(STATE_STRONG_PUNCH, gData.strongPunchTextures, gData.strongPunchAnimation, gData.strongPunchCollisionAnimation, &gData.strongPunchCollisionData);
}

static void setWeakPunch() {
		setPunch(STATE_WEAK_PUNCH, gData.weakPunchTextures, gData.weakPunchAnimation, gData.weakPunchCollisionAnimation, &gData.weakPunchCollisionData);

}

static void checkPunch()  {
	if(gData.state != STATE_IDLE && gData.state != STATE_WALKING) return;

	if(hasPressedXFlank()) {
		if(gData.comboState) setStrongPunch();
		else setWeakPunch();
	}
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
