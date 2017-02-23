#include "userinterface.h"

#include <tari/animation.h>

static int UI_Z_BACK = 10;
static int UI_Z_FRONT = 11;

static struct {
	Position* screenPositionReference;

	TextureData shadowTexture;
	TextureData healthBackground;
	TextureData health;
	int healthID;
} gData;

static void showHealth() {
	Position p = makePosition(14, 23, UI_Z_BACK);
	playAnimationLoop(p, &gData.healthBackground, createOneFrameAnimation(), makeRectangleFromTexture(gData.healthBackground));

	p = makePosition(14+37, 23+3, UI_Z_FRONT);
	gData.healthID = playAnimationLoop(p, &gData.health, createOneFrameAnimation(), makeRectangleFromTexture(gData.health));
	setHealthBarPercentage(1.0);
}

void loadUserInterface() {
	gData.screenPositionReference = NULL;
	gData.shadowTexture = loadTexture("/sprites/SHADOW.pkg");
	gData.healthBackground = loadTexture("/sprites/UI_BACK.pkg");
	gData.health = loadTexture("/sprites/LIFEBAR.pkg");

	showHealth();
}

int addShadow(Position* pos, Position center) {
	Position p = makePosition(-(gData.shadowTexture.mTextureSize.x / 2) + 10, 126-(gData.shadowTexture.mTextureSize.y / 2), -1);
	p = vecAdd(p, center);

	int id = playAnimationLoop(p, &gData.shadowTexture, createOneFrameAnimation(), makeRectangleFromTexture(gData.shadowTexture));
	setAnimationScreenPositionReference(id, gData.screenPositionReference);
	setAnimationBasePositionReference(id, pos);
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


