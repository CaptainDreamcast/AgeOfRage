#include "gamescreen.h"

#include <tari/file.h>
#include <tari/datastructures.h>
#include <tari/wrapper.h>
#include <tari/input.h>
#include <tari/memoryhandler.h>
#include <tari/collisionhandler.h>

#include "system.h"
#include "stage.h"
#include "player.h"
#include "enemies.h"
#include "collision.h"
#include "userinterface.h"
#include "gamestate.h"
#include "titlescreen.h"
#include "continuescreen.h"
#include "congratsscreen.h"

static void loadGameScreen() {
	
	char levelpath[100];

	sprintf(levelpath, "/assets/%s", getCurrentLevelName());
	
	//activateCollisionHandlerDebugMode();

	resetLevelState();
	loadCollision();
	setWorkingDirectory("/assets/player");
	loadPlayer();
	setWorkingDirectory(levelpath);
	loadEnemies();
	setWorkingDirectory("/assets/ui");
	loadUserInterface();
	setWorkingDirectory(levelpath);
	loadStage();
		

}

static void unloadGameScreen() {
	resumeGame();	
}

static void updateGameScreen() {
	updateStage();
	updateEnemies();
	updatePlayer();
	updateUserInterface();
}

static void drawGameScreen() {
	
}

static Screen* getGameScreenNextScreen() {
	
	if(hasPressedAbortFlank()) {
		return &TitleScreen;
	}
	
	if(hasClearedGame()) {
		return &CongratsScreen;
	}
	
	if(hasClearedLevel()) {
		return &GameScreen;
	}
	
	if(hasLost()) {
		return &ContinueScreen;
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

