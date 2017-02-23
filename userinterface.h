#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <tari/geometry.h>

void loadUserInterface();

int addShadow(Position* pos, Position center);
void removeShadow(int id);
void setUserInterfaceScreenPositionReference(Position* pos);
void setHealthBarPercentage(double t);

#endif
