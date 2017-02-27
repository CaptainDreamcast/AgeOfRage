#include "gamestate.h"

#include <stdlib.h>
#include <string.h>

static struct {
	int health;
	
	int isLevelCleared;
	int hasLost;
	char currentLevelName[100];

} gData;

void resetGameState() {
	resetHealth();
	gData.isLevelCleared = 0;
	gData.hasLost = 0;
}

void resetLevelState() {
	gData.isLevelCleared = 0;
	gData.hasLost = 0;
}

int hasClearedLevel() {
	return gData.isLevelCleared;
}

void setLevelCleared(int remainingHealth) {
	gData.isLevelCleared = 1;
	gData.health = remainingHealth;
}

int hasLost() {
	return gData.hasLost;
}

void setLost() {
	gData.hasLost = 1;
}

void resetHealth() {
	gData.health = 1000;
}

int getRemainingHealth(){
	return gData.health;
}

void setCurrentLevelName(char* name) {
	strcpy(gData.currentLevelName, name);
}

char* getCurrentLevelName() {
	return gData.currentLevelName;
}
