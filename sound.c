#include "sound.h"

#include <tari/soundeffect.h>

static struct {
	int punchSFX;

} gData;

void loadSound() {
	gData.punchSFX = loadSoundEffect("/assets/sounds/PUNCH.wav");

}
void playHitSoundEffect() {
	playSoundEffect(gData.punchSFX);
}

