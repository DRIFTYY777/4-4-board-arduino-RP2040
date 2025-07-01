#include <Arduino.h>
#include "mbed.h"
#include "USBKeyboard.h"

USBKeyboard key;

// Rotary Encoder
#define RT_BTN 17
#define ENC_A 15
#define ENC_B 14

// Keypad Matrix
#define ROW1 28
#define ROW2 27
#define ROW3 26
#define ROW4 22

#define COL1 0
#define COL2 1
#define COL3 2
#define COL4 3

constexpr int rowPins[4] = {ROW1, ROW2, ROW3, ROW4};
constexpr int colPins[4] = {COL1, COL2, COL3, COL4};

char keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Encoder state
int prevA = -1;
int prevB = -1;

// Mute debounce state
bool prevMuteState = HIGH;
unsigned long lastMuteTime = 0;

void checkEncoder() {
    int a = digitalRead(ENC_A);
    int b = digitalRead(ENC_B);

    if (a != prevA) {
        if (b != a) {
            // Clockwise - Volume Up
            key.media_control(KEY_VOLUME_UP);
            Serial.println("Volume Up");
        } else {
            // Counter-clockwise - Volume Down
            key.media_control(KEY_VOLUME_DOWN);
            Serial.println("Volume Down");
        }
    }

    prevA = a;
    prevB = b;
}

void checkEncoderButton() {
    bool currentMuteState = digitalRead(RT_BTN);
    if (currentMuteState == LOW && prevMuteState == HIGH) {
        if (millis() - lastMuteTime > 300) {
            key.media_control(KEY_MUTE);
            lastMuteTime = millis();
            Serial.println("Mute");
        }
    }
    prevMuteState = currentMuteState;
}

void checkKeypad() {
    for (int r = 0; r < 4; r++) {
        digitalWrite(rowPins[r], LOW);
        for (int c = 0; c < 4; c++) {
            if (digitalRead(colPins[c]) == LOW) {
                Serial.print("Keypad Pressed: ");
                Serial.println(keymap[r][c]);
                key.printf("%c", keymap[r][c]);
                delay(200); // simple debounce
            }
        }
        digitalWrite(rowPins[r], HIGH);
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(ENC_A, INPUT);
    pinMode(ENC_B, INPUT);
    pinMode(RT_BTN, INPUT_PULLUP);

    for (const int rowPin : rowPins) {
        pinMode(rowPin, OUTPUT);
        digitalWrite(rowPin, HIGH);
    }
    for (const int colPin : colPins) {
        pinMode(colPin, INPUT_PULLUP);
    }

    prevA = digitalRead(ENC_A);
    prevB = digitalRead(ENC_B);
}

void loop() {
    checkEncoder();
    checkEncoderButton();
    checkKeypad();
}
