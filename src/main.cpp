#include <Arduino.h>

#include "mbed.h"
#include "USBKeyboard.h"

USBKeyboard key;

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


void setup() {
    Serial.begin(115200);

    for (const int rowPin : rowPins) {
        pinMode(rowPin, OUTPUT);
        digitalWrite(rowPin, HIGH);
    }
    for (const int colPin : colPins) {
        pinMode(colPin, INPUT_PULLUP);
    }
}

void loop() {
    for (int r = 0; r < 4; r++) {
        digitalWrite(rowPins[r], LOW);
        for (int c = 0; c < 4; c++) {
            if (digitalRead(colPins[c]) == LOW) {

                Serial.println(keymap[r][c]);

                key.printf("%c", keymap[r][c]);

                delay(50); // debounce
            }
        }
        digitalWrite(rowPins[r], HIGH);
    }
}


