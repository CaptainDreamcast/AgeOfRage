#include <kos.h> 

#include <tari/framerateselectscreen.h>
#include <tari/pvr.h>
#include <tari/physics.h>
#include <tari/file.h>
#include <tari/drawing.h>
#include <tari/log.h>
#include <tari/wrapper.h>

#include "gamescreen.h"
#include "system.h"

KOS_INIT_FLAGS(INIT_DEFAULT);

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

uint32_t useRomDisk = 1;

#define DEVELOP

void exitGame() {
  shutdownTariWrapper();

#ifdef DEVELOP
  arch_exit();
#else
  arch_menu();
#endif
}

void setMainFileSystem() {
	#ifdef DEVELOP
		setGameBaseFileSystem("/pc");
	#else
		setGameBaseFileSystem("/cd");
	#endif

	resetToGameBaseFileSystem();
}

int main() {

  initTariWrapperWithDefaultFlags();
  setMainFileSystem();

  log("Check framerate");
  FramerateSelectReturnType framerateReturnType = selectFramerate();
  if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
    exitGame();
  }

  setCurrentLevelName("LEVEL1");
  startScreenHandling(&GameScreen);

  exitGame();


  return (1);
}
