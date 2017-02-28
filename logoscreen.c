#include "logoscreen.h"

#include <tari/file.h>
#include <tari/texture.h>
#include <tari/animation.h>
#include <tari/input.h>
#include <tari/timer.h>

#include "gamestate.h"
#include "titlescreen.h"

static struct {

	TextureData background;
	int isCounterExpired;
	
} gData;

static void countdownFinished(void* caller);


static void loadLogoScreen() {
	gData.isCounterExpired = 0;


	setWorkingDirectory("/assets/logo/");
	gData.background = loadTexture("DOGMA.pkg");
	playAnimationLoop(makePosition(0, 0, 2), &gData.background, createOneFrameAnimation(), makeRectangleFromTexture(gData.background));

	addTimerCB(600, countdownFinished, NULL);
}



static void unloadLogoScreen() {
		
}

static void countdownFinished(void* caller) {
	(void) caller;
	
	gData.isCounterExpired = 1;

}

static void updateLogoScreen() {

}

static void drawLogoScreen() {
	
}

static Screen* getLogoScreenNextScreen() {
	
	if(hasPressedAbortFlank()) {
		abortScreenHandling();
	}
	else if(hasPressedStartFlank()) {
		return &TitleScreen;	
	}

	if(gData.isCounterExpired) {
		return &TitleScreen;
	}

	return NULL;
}


Screen LogoScreen =  {
	.mLoad = loadLogoScreen,
	.mUpdate = updateLogoScreen,
	.mDraw = drawLogoScreen,
	.mUnload = unloadLogoScreen,
	.mGetNextScreen = getLogoScreenNextScreen
};

