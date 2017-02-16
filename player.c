#include "player.h"

#include <tari/system.h>
#include <tari/log.h>
#include <tari/animation.h>
#include <tari/texture.h>
#include <tari/script.h>
#include <tari/physicshandler.h>
#include <tari/input.h>

#include "collision.h"

typedef enum {
	STATE_IDLE, 
	STATE_WALKING,
	STATE_
		
	

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

	int animationID;

	State state;
	int direction;
	
	CollisionData collisionData;
} gData;


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

	gData.animationID = playAnimationLoop(makePosition(0,0,0), gData.idleTextures, gData.idleAnimation, makeRectangleFromTexture(gData.idleTextures[0]));
	setAnimationBasePositionReference(gData.animationID, gData.mPosition);
	setAnimationCenter(gData.animationID, gData.mCenter);

	setHandledPhysicsMaxVelocity(gData.physicsID, 3);
	setHandledPhysicsDragCoefficient(gData.physicsID, makePosition(0.2, 0.2, 0));
}

static void invert() {
	gData.direction *= -1;
	inverseAnimationVertical(gData.animationID);
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

void updatePlayer() {
	checkInverted();
	checkMovement();
}



Position getPlayerPosition() {
	return *gData.mPosition;
}

void setPlayerScreenPositionReference(Position* p) {
	setAnimationScreenPositionReference(gData.animationID, p);
}
