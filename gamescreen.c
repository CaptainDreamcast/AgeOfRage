#include "gamescreen.h"

#include <stdlib.h>
#include <tari/file.h>
#include <tari/datastructures.h>

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
	sprintf(imgpath, "/assets/level/%s.img", gData.currentLevelName);
	mountRomdisk("/assets/level/%s.img", "/LEVEL");
	setFileSystem("/LEVEL");

	
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
	
	return NULL;
}


Screen GameScreen =  {
	.mLoad = loadGameScreen,
	.mUpdate = updateGameScreen,
	.mDraw = drawGameScreen,
	.mUnload = unloadGameScreen,
	.mGetNextScreen = getGameScreenNextScreen
};

