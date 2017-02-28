#include "userinterface.h"

#include <tari/animation.h>
#include <tari/input.h>
#include <tari/wrapper.h>

#include "gamestate.h"

static int UI_Z_BACK = 10;
static int UI_Z_FRONT = 11;
static int UI_Z_PAUSE = 12;

static struct {
	Position* screenPositionReference;

	TextureData shadowTexture;
	TextureData healthBackground;
	TextureData health;
	TextureData pause;
	
	int pauseID;

	int healthID;
} gData;

static void showHealth() {
	Position p = makePosition(14, 23, UI_Z_BACK);
	playAnimationLoop(p, &gData.healthBackground, createOneFrameAnimation(), makeRectangleFromTexture(gData.healthBackground));

	p = makePosition(14+37, 23+3, UI_Z_FRONT);
	gData.healthID = playAnimationLoop(p, &gData.health, createOneFrameAnimation(), makeRectangleFromTexture(gData.health));
	setHealthBarPercentage(getRemainingHealth() / 1000.0);
}

void loadUserInterface() {
	gData.screenPositionReference = NULL;
	gData.shadowTexture = loadTexture("sprites/SHADOW.pkg");
	gData.healthBackground = loadTexture("sprites/UI_BACK.pkg");
	gData.health = loadTexture("sprites/LIFEBAR.pkg");
	gData.pause = loadTexture("sprites/PAUSE.pkg");
	gData.pauseID = -1;

	showHealth();
}

static void pauseGame() {
	pauseWrapper();
	gData.pauseID = playAnimationLoop(makePosition(70, 70, UI_Z_PAUSE), &gData.pause, createOneFrameAnimation(), makeRectangleFromTexture(gData.pause));
}

void resumeGame() {
	if(gData.pauseID == -1) return;
	removeHandledAnimation(gData.pauseID);
	gData.pauseID = -1;
	resumeWrapper();
}

void updateUserInterface() {
	if(hasPressedStartFlank()) {
		if(gData.pauseID == -1) pauseGame();
		else resumeGame();
	}
}

int addShadow(Position* pos, Position center, double scaleX) {
	Position p = makePosition(-(gData.shadowTexture.mTextureSize.x / 2) + 10, 126-(gData.shadowTexture.mTextureSize.y / 2), -1);
	p = vecAdd(p, center);

	int id = playAnimationLoop(p, &gData.shadowTexture, createOneFrameAnimation(), makeRectangleFromTexture(gData.shadowTexture));
	setAnimationScreenPositionReference(id, gData.screenPositionReference);
	setAnimationBasePositionReference(id, pos);
	setAnimationScale(id, makePosition(scaleX, 1, 1), makePosition(32, 0, 0));
	return id;
}

void removeShadow(int id) {
	removeHandledAnimation(id);

}



void setUserInterfaceScreenPositionReference(Position* pos) {
	gData.screenPositionReference = pos;
}


void setHealthBarPercentage(double t) {
	double sizeX = 86*t;
	setAnimationScale(gData.healthID, makePosition(sizeX / 16.0, 8 / 16.0, 1), makePosition(0,0,0));
}


