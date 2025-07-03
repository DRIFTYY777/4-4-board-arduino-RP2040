// keypadAnimation.cpp
#include "keypadAnimation.h"
#include <chrono>

using namespace std::chrono;

int keypadAnimation::getDistance(uint8_t key1, uint8_t key2) {
    int x1 = key1 % 4, y1 = key1 / 4;
    int x2 = key2 % 4, y2 = key2 / 4;
    return max(abs(x1 - x2), abs(y1 - y2));
}

void keypadAnimation::fadeOut() const {
    for (uint8_t fade = brightness; fade > 0; fade--) {
        strip->setBrightness(fade);
        strip->neoPixelShow();
        rtos::ThisThread::sleep_for(20ms);
    }
    strip->setBrightness(brightness);
    strip->neoPixelClear();
}

void keypadAnimation::rippleFromKey(uint8_t keyIndex) {
    rgbMutex.lock();

    int x = keyIndex % 4;
    int y = keyIndex / 4;

    for (int wave = 0; wave < 4; wave++) {
        for (int i = 0; i < 16; i++) {
            int dx = abs((i % 4) - x);
            int dy = abs((i / 4) - y);
            int distance = max(dx, dy);

            if (distance == wave) {
                uint8_t intensity = 255 - (wave * 63);
                strip->neoPixelSetValue(i, 0, intensity, intensity);
            }
        }
        strip->neoPixelShow();
        rtos::ThisThread::sleep_for(50ms);
    }

    for (uint8_t fade = brightness; fade > 0; fade -= 5) {
        strip->setBrightness(fade);
        strip->neoPixelShow();
        rtos::ThisThread::sleep_for(20ms);
    }

    strip->setBrightness(brightness);
    strip->neoPixelClear();
    rgbMutex.unlock();
}

void keypadAnimation::breathingEffect() {

    const unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime < static_cast<unsigned long>(ANIMATION_DELAY)) return;

    static uint8_t breath = 0;
    static int8_t breathDir = 1;

    rgbMutex.lock();
    breath += breathDir * 5;
    if (breath >= brightness || breath <= 0) breathDir *= -1;

    for (int i = 0; i < 16; i++) {
        strip->neoPixelSetValue(i, 0,  0, breath);
    }
    strip->neoPixelShow();
    rgbMutex.unlock();

    lastUpdateTime = currentTime;
}

