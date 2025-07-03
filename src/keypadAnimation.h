// keypadAnimation.h
#pragma once

#include <NeoPixelConnect.h>
#include <mbed.h>

class keypadAnimation {
private:
    // Pointer to the NeoPixelConnect object
    NeoPixelConnect* strip;
    // Animation parameters
    uint8_t brightness;
    // Animation state
    unsigned long lastUpdateTime = 0;
    // Animation constants
    static constexpr int ANIMATION_DELAY = 30;

    // Mutex for thread safety
    rtos::Mutex rgbMutex; // Add mutex as a class member

    // Helper function to set the color of a specific key
    static int getDistance(uint8_t key1, uint8_t key2);

    // Helper function to set the color of a specific key
    void fadeOut() const;

public:

    keypadAnimation(NeoPixelConnect* ledStrip, uint8_t bright = 5) :
        strip(ledStrip), brightness(bright) {
        strip->setBrightness(brightness);
    }

    void rippleFromKey(uint8_t keyIndex);
    void breathingEffect();


};