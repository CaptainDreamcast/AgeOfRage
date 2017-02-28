#include "continuescreen.h"

#include <tari/file.h>
#include <tari/texture.h>
#include <tari/animation.h>
#include <tari/input.h>
#include <tari/physicshandler.h>

#include "titlescreen.h"

static struct {
	TextureData background;
	TextureData text;
	TextureData congrats;
	TextureData thanks;

	int textPhysicsID;
	int textID;	

} gData;

static void loadCongratsScreen() {
	setWorkingDirectory("/assets/congrats/");
	gData.background = loadTexture("BG.pkg");
	playAnimationLoop(makePosition(0, 0, 2), &gData.background, createOneFrameAnimation(), makeRectangleFromTexture(gData.background));
	
	gData.text = loadTexture("CONGRATSTEXT.pkg");
	gData.textPhysicsID = addToPhysicsHandler(makePosition(0, 260, 3));
	addAccelerationToHandledPhysics(gData.textPhysicsID, makePosition(0, -0.3, 0));
	gData.textID = playAnimationLoop(makePosition(0, 0, 0), &gData.text, createOneFrameAnimation(), makeRectangleFromTexture(gData.text));
	PhysicsObject* physics = getPhysicsFromHandler(gData.textPhysicsID);
	setAnimationBasePositionReference(gData.textID, &physics->mPosition);
	
	gData.congrats = loadTexture("CONGRATS.pkg");
	playAnimationLoop(makePosition(0, 0, 4), &gData.congrats, createOneFrameAnimation(), makeRectangleFromTexture(gData.congrats));


	gData.thanks = loadTexture("THANKS.pkg");	
}



static void unloadCongratsScreen() {
		
}

static void showThanks() {
	removeAnimationCB(gData.textID);
	removeFromPhysicsHandler(gData.textPhysicsID);
	gData.textPhysicsID = gData.textID = -1;

	playAnimationLoop(makePosition(0, 0, 3), &gData.thanks, createOneFrameAnimation(), makeRectangleFromTexture(gData.thanks));

}


static void updateCongratsScreen() {
	if(gData.textPhysicsID != -1) {
		PhysicsObject* physics = getPhysicsFromHandler(gData.textPhysicsID);
		if(physics->mPosition.y < -200) {
			showThanks();
		}
	}
}

static void drawCongratsScreen() {
	
}

static Screen* getCongratsScreenNextScreen() {
	
	if(hasPressedAbortFlank()) {
		return &TitleScreen;		
	}
	else if(hasPressedStartFlank()) {
		return &TitleScreen;
	}


	return NULL;
}


Screen CongratsScreen =  {
	.mLoad = loadCongratsScreen,
	.mUpdate = updateCongratsScreen,
	.mDraw = drawCongratsScreen,
	.mUnload = unloadCongratsScreen,
	.mGetNextScreen = getCongratsScreenNextScreen
};

