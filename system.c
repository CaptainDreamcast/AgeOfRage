#include "system.h"

#include <tari/file.h>

static struct {
	char fs[100];
} gData;

void setGameBaseFileSystem(char* fs) {
	strcpy(gData.fs, fs);
}

void resetToGameBaseFileSystem() {
	setFileSystem(gData.fs);
}
