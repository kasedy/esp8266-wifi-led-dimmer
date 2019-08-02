#pragma once

#include <vector>
#include <animations/all.h>

Effect noAnimation() {
  return NoAnimation::effect("No animation");
}

Effect effectFadeOnSwitch() {
  return FadeSwitch::effect("Fade on switch");
}

Effect effectFadeInCycle() {
  return FadeCycle::effect("Fade in cycle");
}

Effect effectFadeSingleLed() {
  return SingleLedFade::effect("Fade single led", 255);
}

Effect effectRandomSplashes() {
  return RandomSplashes::effect("Random splashes");
}

Effect effectRandomCompensatedSplashes() {
  return RandomSplashesCompensated::effect("Random compensated splashes");
}

Effect effectRandomBlinks() {
  return RandomSplashes::effect("Random blinks", {{0, 0}, {255, 255}});
}

std::vector<Effect> defaultEffects() {
  return std::vector<Effect>({
    effectRandomCompensatedSplashes(),
    effectFadeOnSwitch(),
    effectFadeSingleLed(),
  });
}