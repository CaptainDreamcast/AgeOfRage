#include "gamescreen.h"

#include <stdlib.h>
#include <tari/file.h>
#include <tari/datastructures.h>
#include <tari/wrapper.h>
#include <tari/input.h>
#include <tari/memoryhandler.h>


#include "system.h"
#include "stage.h"
#include "player.h"
#include "enemies.h"
#include "collision.h"

static struct {
	char currentLevelName[100];
} gData;

void setCurrentLevelName(char* name) {
	strcpy(gData.currentLevelName, name);
}

static void loadGameScreen() {
	
	char imgpath[100];

	sprintf(imgpath, "/assets/level/%s", gData.currentLevelName);
	setFileSystem("/pc/assets/level/level1");

	//printDirectory("scripts");

	loadCollision();
	loadPlayer();
	loadEnemies();
	loadStage();
	
}

static void unloadGameScreen() {
	resetToGameBaseFileSystem();
	unmountRomdisk("/LEVEL");	
}

static void updateGameScreen() {
	updateStage();
	updateEnemies();
	updatePlayer();
}

static void drawGameScreen() {
	
}

static Screen* getGameScreenNextScreen() {
	
	if(hasPressedAbortFlank()) {
		abortScreenHandling();
	}

	return NULL;
}


Screen GameScreen =  {
	.mLoad = loadGameScreen,
	.mUpdate = updateGameScreen,
	.mDraw = drawGameScreen,
	.mUnload = unloadGameScreen,
	.mGetNextScreen = getGameScreenNextScreen
};

