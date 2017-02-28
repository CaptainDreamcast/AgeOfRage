#include "titlescreen.h"


#include <tari/file.h>
#include <tari/texture.h>
#include <tari/animation.h>
#include <tari/input.h>
#include <tari/timer.h>

#include "gamestate.h"
#include "gamescreen.h"
#include "titlescreen.h"
#include "sound.h"

static struct {

	TextureData background;
	TextureData player[2][10];
	Animation playerAnimation[2];
	TextureData enemy[4][2][10];
	Animation enemyAnimation[4][2];

	int playerID;
	int enemyID;

	int whichEnemy;
	int whichEnemyAnimation;

} gData;


static void loadAnimationAndTextures(int frameAmount, int duration, char* name, TextureData* tData, Animation* anim) {
	char path[300];

	*anim = createEmptyAnimation();
	anim->mFrameAmount = frameAmount;
	anim->mDuration = duration;
	
	int i;
	for(i = 0; i < frameAmount; i++) {
		sprintf(path, "%s%d.pkg", name, i);
		tData[i] = loadTexture(path);
	}

}

static void playerAnimationFinished(void* caller);

static void enemyAnimationFinished(void* caller);

static void loadTitleScreen() {

	setWorkingDirectory("/assets/title/");
	gData.background = loadTexture("BG.pkg");
	playAnimationLoop(makePosition(0, 0, 2), &gData.background, createOneFrameAnimation(), makeRectangleFromTexture(gData.background));
	
	setWorkingDirectory("/assets/player/sprites/");	
	loadAnimationAndTextures(4, 10, "IDLE", gData.player[0], &gData.playerAnimation[0]);
	loadAnimationAndTextures(5, 4, "PUNCH1_", gData.player[1], &gData.playerAnimation[1]);

	setWorkingDirectory("/assets/enemies/sprites/");	
	loadAnimationAndTextures(4, 10, "SALARYMAN_IDLE", gData.enemy[0][0], &gData.enemyAnimation[0][0]);
	loadAnimationAndTextures(8, 6, "SALARYMAN_DEATH", gData.enemy[0][1], &gData.enemyAnimation[0][1]);
	loadAnimationAndTextures(1, 10, "TIGER_IDLE", gData.enemy[1][0], &gData.enemyAnimation[1][0]);
	loadAnimationAndTextures(1, 60, "TIGER_HIT", gData.enemy[1][1], &gData.enemyAnimation[1][1]);
	loadAnimationAndTextures(4, 10, "GRANNY_IDLE", gData.enemy[2][0], &gData.enemyAnimation[2][0]);
	loadAnimationAndTextures(8, 6, "GRANNY_DEATH", gData.enemy[2][1], &gData.enemyAnimation[2][1]);
	loadAnimationAndTextures(4, 10, "MIRKLING_IDLE", gData.enemy[3][0], &gData.enemyAnimation[3][0]);
	loadAnimationAndTextures(8, 6, "MIRKLING_DEATH", gData.enemy[3][1], &gData.enemyAnimation[3][1]);
	
	
	gData.playerID = playAnimationLoop(makePosition(70, 100, 4), gData.player[0], gData.playerAnimation[0], makeRectangleFromTexture(gData.player[0][0]));
	setAnimationCB(gData.playerID, playerAnimationFinished, NULL);
	
	gData.enemyID = playAnimationLoop(makePosition(105, 100, 3), gData.enemy[0][0], gData.enemyAnimation[0][0], makeRectangleFromTexture(gData.enemy[0][0][0]));
	setAnimationCB(gData.enemyID, enemyAnimationFinished, NULL);
	setAnimationCenter(gData.enemyID, makePosition(60, 0, 0));
	inverseAnimationVertical(gData.enemyID);

	gData.whichEnemy = 0;
	gData.whichEnemyAnimation = 0;

	loadSound();
}


static void unloadTitleScreen() {
		
}


static void punchHasConnected(void* caller) {
	gData.whichEnemyAnimation = 1;
	changeAnimation(gData.enemyID, gData.enemy[gData.whichEnemy][gData.whichEnemyAnimation], gData.enemyAnimation[gData.whichEnemy][gData.whichEnemyAnimation], makeRectangleFromTexture(gData.enemy[gData.whichEnemy][gData.whichEnemyAnimation][0]));
	playHitSoundEffect();
}

static void playerAnimationFinished(void* caller) {
	changeAnimation(gData.playerID, gData.player[0], gData.playerAnimation[0], makeRectangleFromTexture(gData.player[0][0]));
}

static void setEnemyPosition() {
	Position p;
	if(gData.whichEnemy == 1) {
		p = makePosition(235, 155, 3);
	} else {
		p = makePosition(105, 100, 3);
	}
	setAnimationPosition(gData.enemyID, p);
}

static void enemyAnimationFinished(void* caller) {
	(void) caller;
	if(gData.whichEnemyAnimation == 1) {
		gData.whichEnemy = (gData.whichEnemy + 1) % 4;		
		gData.whichEnemyAnimation = 0;
		setEnemyPosition();
		changeAnimation(gData.enemyID, gData.enemy[gData.whichEnemy][gData.whichEnemyAnimation], gData.enemyAnimation[gData.whichEnemy][gData.whichEnemyAnimation], makeRectangleFromTexture(gData.enemy[gData.whichEnemy][gData.whichEnemyAnimation][0]));
	} else {
		changeAnimation(gData.playerID, gData.player[1], gData.playerAnimation[1], makeRectangleFromTexture(gData.player[1][0]));
		addTimerCB(2*4, punchHasConnected, NULL);
	}
	
}

static void updateTitleScreen() {

}

static void drawTitleScreen() {
	
}

static Screen* getTitleScreenNextScreen() {
	
	if(hasPressedAbortFlank()) {
		abortScreenHandling();
	}
	else if(hasPressedStartFlank()) {
		resetGameState();
  		setCurrentLevelName("level1");
		return &GameScreen;
	}

	

	return NULL;
}


Screen TitleScreen =  {
	.mLoad = loadTitleScreen,
	.mUpdate = updateTitleScreen,
	.mDraw = drawTitleScreen,
	.mUnload = unloadTitleScreen,
	.mGetNextScreen = getTitleScreenNextScreen
};

