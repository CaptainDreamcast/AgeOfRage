#include <kos.h> 

#include <tari/framerateselectscreen.h>
#include <tari/pvr.h>
#include <tari/physics.h>
#include <tari/file.h>
#include <tari/drawing.h>
#include <tari/log.h>
#include <tari/wrapper.h>

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
	setFileSystem("/pc");
}

int main() {

  initTariWrapperWithDefaultFlags();
  setMainFileSystem();

  log("Check framerate");
  FramerateSelectReturnType framerateReturnType = selectFramerate();
  if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
    exitGame();
  }

  

  exitGame();


  return (1);
}
