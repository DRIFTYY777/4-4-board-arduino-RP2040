#include <Arduino.h>
#include "mbed.h"
#include "USBMouseKeyboard.h"

#include "keypadAnimation.h"

using namespace mbed;
using namespace rtos;

#define RGB_PIN 13 // RGB LED pin
#define LED_COUNT 16
#define BRIGHTNESS 100 // Set brightness to maximum

// Rotary Encoder and Button pin out
#define RT_BTN 17
#define ENC_A 15
#define ENC_B 14

// Keypad Matrix pin out
#define ROW1 0
#define ROW2 1
#define ROW3 2
#define ROW4 3

#define COL1 28
#define COL2 27
#define COL3 26
#define COL4 22

constexpr int rowPins[4] = {ROW4, ROW3, ROW2, ROW1};
constexpr int colPins[4] = {COL4, COL3, COL2, COL1};

// Keypad matrix layout
char keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};


// Message queue for keypad events
struct KeypadEvent {
    uint8_t keyIndex;
    char keyValue;
};


// Previous state of the rotary encoder
int prevA = -1;
int prevB = -1;

/// @brief Rotary Encoder State
enum ROTARY_STATE
{
    SCROLLING = 0,
    VOLUME_CONTROL = 1,
};
ROTARY_STATE currentRotaryState = SCROLLING;

/// Rotary Button State
enum ROTARY_BTN_STATE
{
    PRESSED = 0,
    RELEASED = 1,
    LONG_PRESSED = 2,
};

// Global queue for communication between threads
Queue<KeypadEvent, 16> keypadQueue;
Mutex rgbMutex;

/// USB Mouse and Keyboard instance
USBMouseKeyboard key_mouse;
// NeoPixel instance

NeoPixelConnect RGB(RGB_PIN, LED_COUNT, pio0, 0);// Thread for RGB animation
keypadAnimation* animations;


Thread thread;
Thread keypadThread;

// return the button is pressed, long pressed or released
ROTARY_BTN_STATE checkRotaryButton()
{

    static unsigned long lastPressTime = 0;
    static bool longPressHandled = false;
    const bool currentState = digitalRead(RT_BTN);

    if (currentState == LOW)
    { // Button pressed
        if (lastPressTime == 0)
        {
            lastPressTime = millis();
            longPressHandled = false;
            return PRESSED;
        }
        else if (millis() - lastPressTime > 1000 && !longPressHandled)
        {
            longPressHandled = true;
            return LONG_PRESSED;
        }
    }
    else
    { // Button released
        lastPressTime = 0;
        longPressHandled = false;
        return RELEASED;
    }
    return RELEASED;
}

// return 2 value via pointer
void EncoderValue(int *a, int *b)
{
    const int enc_a = digitalRead(ENC_A);
    const int enc_b = digitalRead(ENC_B);
    *a = 0;
    *b = 0;
    if (enc_a != prevA)
    {
        if (enc_b != enc_a)
        {
            *a = enc_a; // Clockwise
        }
        else
        {
            *b = enc_b; // Counter-clockwise
        }
    }
    prevA = enc_a;
    prevB = enc_b;
}

void scrollMouse()
{
    int a, b;
    EncoderValue(&a, &b);
    if (a)
    {
        Serial.print("Mouse Scrolling: ");
        Serial.println(a);
        key_mouse.scroll(static_cast<int8_t>(a));
    }
    else if (b)
    {
        Serial.print("Mouse Scrolling: ");
        Serial.println(-b);
        key_mouse.scroll(static_cast<int8_t>(-b));
    }
}

void volumeControl()
{
    int a, b;
    EncoderValue(&a, &b);
    if (a)
    {
        Serial.print("Volume Up: ");
        Serial.println(a);
        key_mouse.media_control(KEY_VOLUME_UP);
    }
    else if (b)
    {
        Serial.print("Volume Down: ");
        Serial.println(-b);
        key_mouse.media_control(KEY_VOLUME_DOWN);
    }
}

// checkKeypad function
void checkKeypad() {
    for (int r = 0; r < 4; r++) {
        digitalWrite(rowPins[r], LOW);
        for (int c = 0; c < 4; c++) {
            if (digitalRead(colPins[c]) == LOW) {

                KeypadEvent* event = new KeypadEvent();

                event->keyIndex = r * 4 + c;
                event->keyValue = keymap[r][c];

                // Send event to queue
                keypadQueue.try_put(event);
                delay(200); // debounce
            }
        }
        digitalWrite(rowPins[r], HIGH);
    }
}


// Blink the built-in LED to indicate the current state
void blinkLED(const bool state)
{
    digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
}

// Keypad processing thread
[[noreturn]] void keypad_thread() {

    KeypadEvent* event;

    while (true) {
        if (keypadQueue.try_get(&event)) {
            // Process keypress

            key_mouse.printf("%c", event->keyValue);

            animations->rippleFromKey(event->keyIndex);

            delete event; // Free the allocated memory
        }
        ThisThread::sleep_for(10);
    }
}

// create a thread for the NeoPixel animation
[[noreturn]] void RGB_THREAD() {
    while (true) {
        animations->breathingEffect();
        ThisThread::sleep_for(50);
    }
}



void setup()
{
    Serial.begin(115200);

    RGB.neoPixelInit(RGB_PIN, LED_COUNT);
    animations = new keypadAnimation(&RGB, BRIGHTNESS);

    // Initialize pins
    pinMode(LED_BUILTIN, OUTPUT);
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

    // Start threads
    thread.start(RGB_THREAD);
    keypadThread.start(keypad_thread);
}

void loop()
{

    ROTARY_BTN_STATE btnState = checkRotaryButton();

    // Handle rotary state switching
    if (btnState == LONG_PRESSED)
    {
        currentRotaryState = (currentRotaryState == SCROLLING) ? VOLUME_CONTROL : SCROLLING;
        // Blink LED to indicate state change
        blinkLED(currentRotaryState == VOLUME_CONTROL);
    }
    else if (btnState == PRESSED)
    {
        // Short press for mute in volume mode
        if (currentRotaryState == VOLUME_CONTROL)
        {
            key_mouse.media_control(KEY_MUTE);
            Serial.println("Mute");
        }
    }

    // Handle rotary functions based on a current state
    if (currentRotaryState == SCROLLING)
    {
        scrollMouse();
    }
    else
    {
        volumeControl();
    }

    checkKeypad();
}
