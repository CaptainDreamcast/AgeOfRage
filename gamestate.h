#ifndef GAMESTATE_H
#define GAMESTATE_H

void resetGameState();
void resetLevelState();
int hasClearedLevel();
void setLevelCleared(int remainingHealth);
int hasLost();
void setLost();
int getRemainingHealth();
void resetHealth();

void setCurrentLevelName(char* name);
char* getCurrentLevelName();

#endif
