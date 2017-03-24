#include <tari/framerateselectscreen.h>
#include <tari/pvr.h>
#include <tari/physics.h>
#include <tari/file.h>
#include <tari/drawing.h>
#include <tari/log.h>
#include <tari/wrapper.h>
#include <tari/system.h>

#include "gamescreen.h"
#include "titlescreen.h"
#include "continuescreen.h"
#include "congratsscreen.h"
#include "gameoverscreen.h"
#include "gamestate.h"
#include "system.h"
#include "logoscreen.h"

#ifdef DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);
#endif


void exitGame() {
  shutdownTariWrapper();

#define DEVELOP

#ifdef DEVELOP
  abortSystem();
#else
  returnToMenu();
#endif
}

void setMainFileSystem() {
	#ifdef DEVELOP
		setGameBaseFileSystem("/pc");
	#else
		setGameBaseFileSystem("/rd");
	#endif

	resetToGameBaseFileSystem();
}

int main(int argc, char** argv) {

  setGameName("FISTS OF JUSTICE");
  setScreenSize(320, 240);

  initTariWrapperWithDefaultFlags();

  logg("Check framerate");
  FramerateSelectReturnType framerateReturnType = selectFramerate();
  if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
    exitGame();
  }

  
  setMainFileSystem();

  resetGameState();
  setCurrentLevelName("level4");
  startScreenHandling(&LogoScreen);

  exitGame();


  return (1);
}


// TODO: find out why main doesn't work
int wmain(int argc, char** argv) {
	return main(argc, argv);
}


