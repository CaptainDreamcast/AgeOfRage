#include "gameoverscreen.h"

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


static void loadGameOverScreen() {
	gData.isCounterExpired = 0;


	setWorkingDirectory("/assets/gameover/");
	gData.background = loadTexture("BG.pkg");
	playAnimationLoop(makePosition(0, 0, 2), &gData.background, createOneFrameAnimation(), makeRectangleFromTexture(gData.background));

	addTimerCB(600, countdownFinished, NULL);
}



static void unloadGameOverScreen() {
		
}

static void countdownFinished(void* caller) {
	(void) caller;
	
	gData.isCounterExpired = 1;

}

static void updateGameOverScreen() {

}

static void drawGameOverScreen() {
	
}

static Screen* getGameOverScreenNextScreen() {
	
	if(hasPressedAbortFlank()) {
		return &TitleScreen;		
	}
	else if(hasPressedStartFlank()) {
		return &TitleScreen;	
	}

	if(gData.isCounterExpired) {
		return &TitleScreen;
	}

	return NULL;
}


Screen GameOverScreen =  {
	.mLoad = loadGameOverScreen,
	.mUpdate = updateGameOverScreen,
	.mDraw = drawGameOverScreen,
	.mUnload = unloadGameOverScreen,
	.mGetNextScreen = getGameOverScreenNextScreen
};

