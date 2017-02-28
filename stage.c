#include "stage.h"

#include <string.h>
#include <stdio.h>

#include <tari/geometry.h>
#include <tari/stagehandler.h>
#include <tari/script.h>
#include <tari/log.h>
#include <tari/system.h>
#include <tari/collisionhandler.h>
#include <tari/memoryhandler.h>
#include <tari/datastructures.h>
#include <tari/timer.h>

#include "enemies.h"
#include "player.h"
#include "userinterface.h"
#include "gamestate.h"
#include "system.h"

typedef struct  {
	TextureData* textures;
	Animation animation;
} StageAnimation;

static struct {
	Script script;
	ScriptPosition scriptPosition;

	int backgroundID1;
	int backgroundID2;
	int groundID;

	int isWaitingForDefeat;
	int isWaitingForAnimation;
	int isBackgroundScrolling;

	int isOver;
	Position* screenPosition;

	int isScrollingRightForced;

	List animations;

} gData;

static ScriptPosition loadStageAnimation(ScriptPosition position) {
	int frameAmount;
	
	Animation animation = createEmptyAnimation();
	position = getNextScriptInteger(position, &frameAmount);
	position = getNextScriptDouble(position, &animation.mDuration);
	animation.mFrameAmount = frameAmount;

	StageAnimation* e = allocMemory(sizeof(StageAnimation));
	e->animation = animation;
	e->textures = allocMemory(sizeof(TextureData)*frameAmount);

	int i;
	for(i = 0; i < frameAmount; i++) {
		char name[100], path[100];

		position = getNextScriptString(position, name);
		sprintf(path, "sprites/%s", name);
		e->textures[i] = loadTexture(path);
	}
	

	list_push_front_owned(&gData.animations, e);	

	return position;	
}

static ScriptPosition loadBackgroundElementWithoutAnimation(ScriptPosition position, char* path, Position* texturePosition) {	
	char name[100];
	position = getNextScriptString(position, name);
	sprintf(path, "sprites/%s.pkg", name);
		
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
	} else if(!strcmp(word, "ANIMATION")) {
		position = loadStageAnimation(position);
	} else {
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
	gData.isScrollingRightForced = 0;
	gData.isWaitingForAnimation = 0;

	gData.animations = new_list();

	gData.isOver = 0;
	gData.screenPosition = getScrollingBackgroundPositionReference(gData.groundID);

	gData.script = loadScript("scripts/stage.txt");
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

static void blockingAnimationFinished(void* caller) {
	gData.isWaitingForAnimation = 0;
}

static ScriptPosition showAnimation(ScriptPosition pos, int isBlocking) {
	int id;
	Position p;
	gData.scriptPosition = getNextScriptInteger(gData.scriptPosition, &id);
	gData.scriptPosition = getNextScriptDouble(gData.scriptPosition, &p.x);
	gData.scriptPosition = getNextScriptDouble(gData.scriptPosition, &p.y);
	gData.scriptPosition = getNextScriptDouble(gData.scriptPosition, &p.z);
	StageAnimation* data = list_get(&gData.animations, id);
	playAnimation(p, data->textures, data->animation, makeRectangleFromTexture(data->textures[0]), blockingAnimationFinished, NULL);
	gData.isWaitingForAnimation = isBlocking;
	return pos;

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

		if(gData.isWaitingForAnimation) return;

		if(!hasNextScriptWord(gData.scriptPosition)) {
			gData.isOver = 1;
			setLevelCleared(getPlayerHealth());
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
		} else if(!strcmp("LEVEL", word)) {
			char name[100];
			gData.scriptPosition = getNextScriptString(gData.scriptPosition, name);
			setCurrentLevelName(name);
		}  else if(!strcmp("FREEZE_PLAYER", word)) {
			freezePlayer();
		} else if(!strcmp("UNFREEZE_PLAYER", word)) {
			unfreezePlayer();
		} else if(!strcmp("FREEZE_ENEMY", word)) {
			freezeEnemies();
		} else if(!strcmp("UNFREEZE_ENEMY", word)) {
			unfreezeEnemies();
		} else if(!strcmp("SCROLL_SCREEN_RIGHT", word)) {
			gData.isScrollingRightForced = 1;
		} else if(!strcmp("STOP_SCROLL", word)) {
			gData.isScrollingRightForced = 0;
		}  else if(!strcmp("BLOCK_SCROLL", word)) {
			gData.isBackgroundScrolling = 0;
		} else if(!strcmp("ANIMATION", word)) {
			pos = showAnimation(pos, 0);
		}  else if(!strcmp("ANIMATION_BLOCKING", word)) {
			pos = showAnimation(pos, 1);
		} else if(!strcmp("WAIT_DURATION", word)) {
			double duration;
			gData.scriptPosition = getNextScriptDouble(gData.scriptPosition, &duration);
			addTimerCB(duration, blockingAnimationFinished, NULL);
			gData.isWaitingForAnimation = 1;
		}  else if(!strcmp("CONGRATS", word)) {
			setGameCleared();
		} else {
			logError("Unknown token");
			logErrorString(word);
			abortSystem();
		}

		
	

		gData.scriptPosition = getNextScriptInstruction(gData.scriptPosition);
	}
}

static void updateStageMovement() {
	if(!gData.isScrollingRightForced && !gData.isBackgroundScrolling) return;

	Position p = getRealScreenPosition(gData.groundID, getPlayerPosition());
	if(gData.isScrollingRightForced || p.x >= 175) {
		scrollBackgroundRight(0.25);
	}
}

void updateStage() {
		updateScript();
		updateStageMovement();
}


