#include "stage.h"

#include <string.h>
#include <stdio.h>

#include <tari/geometry.h>
#include <tari/stagehandler.h>
#include <tari/script.h>
#include <tari/log.h>
#include <tari/system.h>
#include <tari/collisionhandler.h>

#include "enemies.h"
#include "player.h"
#include "userinterface.h"

static struct {
	Script script;
	ScriptPosition scriptPosition;

	int backgroundID1;
	int backgroundID2;
	int groundID;

	int isWaitingForDefeat;
	int isBackgroundScrolling;

	int isOver;
	Position* screenPosition;

} gData;

static ScriptPosition loadBackgroundElementWithoutAnimation(ScriptPosition position, char* path, Position* texturePosition) {	
	char name[100];
	position = getNextScriptString(position, name);
	sprintf(path, "/sprites/%s.pkg", name);
		
	position = getNextScriptDouble(position, &texturePosition->x);
	position = getNextScriptDouble(position, &texturePosition->y);
	return position;
}

static ScriptPosition loadBackgroundElementAnimation(ScriptPosition position, Animation* animation) {	
	resetAnimation(animation);
	int v;
	position = getNextScriptInteger(position, &v);
	animation->mFrameAmount = v;
	position = getNextScriptDouble(position, &animation->mDuration);
	return position;
}

static ScriptPosition loader(void* caller, ScriptPosition position) {
	char word[100];	
	char path[100];
	position = getNextScriptString(position, word);

	if(!strcmp(word, "BACKGROUND1")) {
		Position texturePosition;
		position = loadBackgroundElementWithoutAnimation(position, path, &texturePosition);
		addBackgroundElement(gData.backgroundID1, texturePosition, path, createOneFrameAnimation());
	} else if(!strcmp(word, "BACKGROUND_ANIMATION1")) {
		Position texturePosition;
		position = loadBackgroundElementWithoutAnimation(position, path, &texturePosition);
		Animation animation;		
		position = loadBackgroundElementAnimation(position, &animation);
		addBackgroundElement(gData.backgroundID1, texturePosition, path, animation);
	} else if(!strcmp(word, "BACKGROUND2")) {
		Position texturePosition;
		position = loadBackgroundElementWithoutAnimation(position, path, &texturePosition);
		addBackgroundElement(gData.backgroundID2, texturePosition, path, createOneFrameAnimation());
	}  else if(!strcmp(word, "BACKGROUND_ANIMATION2")) {
		Position texturePosition;
		position = loadBackgroundElementWithoutAnimation(position, path, &texturePosition);
		Animation animation;		
		position = loadBackgroundElementAnimation(position, &animation);
		
		addBackgroundElement(gData.backgroundID2, texturePosition, path, animation);
	}   else {
		logError("Unrecognized token.");
		logErrorString(word);
		abortSystem();
	}

	return position;
}

void loadStage() {
	gData.backgroundID1 = addScrollingBackground(1, 1);
	gData.backgroundID2 = addScrollingBackground(0.75, 2);
	gData.groundID = addScrollingBackground(1, 3);

	gData.isWaitingForDefeat = 0;
	gData.isBackgroundScrolling = 1;

	gData.isOver = 0;
	gData.screenPosition = getScrollingBackgroundPositionReference(gData.groundID);

	gData.script = loadScript("/scripts/stage.txt");
	ScriptRegion r = getScriptRegion(gData.script, "LOAD");
	executeOnScriptRegion(r, loader, NULL);

	gData.scriptPosition = getScriptRegionStart(getScriptRegion(gData.script, "MAIN"));

	Position* p = getScrollingBackgroundPositionReference(gData.groundID);
	setPlayerScreenPositionReference(p);
	setEnemiesScreenPositionReference(p);
	setUserInterfaceScreenPositionReference(p);
	addPlayerShadow();
	setCollisionHandlerDebuggingScreenPositionReference(p);
}

static void updateScript() {

	int isActive = 1;
	while(isActive) {

		if(gData.isWaitingForDefeat) {
			int activeEnemyAmount = getActiveEnemyAmount();
			if(activeEnemyAmount) return;

			gData.isWaitingForDefeat = 0;
			gData.isBackgroundScrolling = 1;
		}

		if(!hasNextScriptWord(gData.scriptPosition)) {
			gData.isOver = 1;
			return;
		}

		int screenPositionX;
		ScriptPosition pos = getNextScriptInteger(gData.scriptPosition, &screenPositionX);
		
		if(gData.screenPosition->x < screenPositionX) return;
		gData.scriptPosition = pos;

		char word[100];
		gData.scriptPosition = getNextScriptString(gData.scriptPosition, word);

		if(!strcmp("WAIT", word)) {
			gData.isWaitingForDefeat = 1;
			gData.isBackgroundScrolling = 0;
		} else if(!strcmp("ENEMY", word)) {
			Position enemyPosition;
			int enemyType;
			gData.scriptPosition = getNextScriptDouble(gData.scriptPosition, &enemyPosition.x);
			gData.scriptPosition = getNextScriptDouble(gData.scriptPosition, &enemyPosition.y);
			gData.scriptPosition = getNextScriptDouble(gData.scriptPosition, &enemyPosition.z);
			gData.scriptPosition = getNextScriptInteger(gData.scriptPosition, &enemyType);
			enemyPosition = vecAdd(enemyPosition, *gData.screenPosition);
			spawnEnemy(enemyType, enemyPosition);	
		}

		gData.scriptPosition = getNextScriptInstruction(gData.scriptPosition);
	}
}

static void updateStageMovement() {
	if(!gData.isBackgroundScrolling) return;

	Position p = getRealScreenPosition(gData.groundID, getPlayerPosition());
	if(p.x >= 175) {
		scrollBackgroundRight(0.25);
	}
}

void updateStage() {
		updateScript();
		updateStageMovement();
}


