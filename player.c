#include "player.h"

#include <tari/system.h>
#include <tari/log.h>
#include <tari/animation.h>
#include <tari/texture.h>
#include <tari/script.h>
#include <tari/physicshandler.h>

static struct {

	int physicsID;	
	Position* mPosition;

	Animation idleAnimation;
	TextureData idleTextures[10];


	Animation walkingAnimation;
	TextureData walkingTextures[10];

	int animationID;
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
	gData.animationID = playAnimationLoop(makePosition(0,0,0), gData.idleTextures, gData.idleAnimation, makeRectangleFromTexture(gData.idleTextures[0]));
	setAnimationBasePositionReference(gData.animationID, gData.mPosition);
}

void updatePlayer() {
 // TODO
}



Position getPlayerPosition() {
	return *gData.mPosition;
}

void setPlayerScreenPositionReference(Position* p) {
	setAnimationScreenPositionReference(gData.animationID, p);
}
