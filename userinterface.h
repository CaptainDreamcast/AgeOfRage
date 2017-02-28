#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <tari/geometry.h>

void loadUserInterface();
void updateUserInterface();
void resumeGame();

int addShadow(Position* pos, Position center, double scaleX);
void removeShadow(int id);
void setUserInterfaceScreenPositionReference(Position* pos);
void setHealthBarPercentage(double t);

#endif
