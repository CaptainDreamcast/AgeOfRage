#include "continuescreen.h"

#include <tari/file.h>
#include <tari/texture.h>
#include <tari/animation.h>
#include <tari/input.h>

#include "gamestate.h"
#include "gamescreen.h"
#include "titlescreen.h"

static struct {

	TextureData background;
	TextureData numbers[11];
	int numberID;
	int isCounterExpired;
	

} gData;

static void countdownFinished(void* caller);


static void loadContinueScreen() {
	gData.isCounterExpired = 0;


	setWorkingDirectory("/assets/continue/");
	gData.background = loadTexture("BG.pkg");
	playAnimationLoop(makePosition(0, 0, 2), &gData.background, createOneFrameAnimation(), makeRectangleFromTexture(gData.background));
	
	int i;
	for(i = 0; i <= 10; i++) {
		char path[100];
		sprintf(path, "NUMBER%d.pkg", i);
		gData.numbers[i] = loadTexture(path);
	}	

	Animation anim = createEmptyAnimation();
	anim.mFrameAmount = 11;
	anim.mDuration = 60;
	gData.numberID = playAnimation(makePosition(100, 100, 3), gData.numbers, anim, makeRectangleFromTexture(gData.numbers[0]), countdownFinished, NULL);
}



static void unloadContinueScreen() {
		
}

static void countdownFinished(void* caller) {
	(void) caller;
	
	gData.isCounterExpired = 1;

}

static void updateContinueScreen() {

}

static void drawContinueScreen() {
	
}

static Screen* getContinueScreenNextScreen() {
	
	if(hasPressedStartFlank()) {
		resetHealth();
		return &GameScreen;
	}

	if(hasPressedAbortFlank()) {
		return &TitleScreen;		
	}

	if(gData.isCounterExpired) {
		return &TitleScreen;
	}

	return NULL;
}


Screen ContinueScreen =  {
	.mLoad = loadContinueScreen,
	.mUpdate = updateContinueScreen,
	.mDraw = drawContinueScreen,
	.mUnload = unloadContinueScreen,
	.mGetNextScreen = getContinueScreenNextScreen
};

