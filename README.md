# 4x4 Keypad with Rotary Encoder (RP2040)

A versatile input device built with the Raspberry Pi Pico (RP2040) featuring a 4x4 matrix keypad and a dual-mode rotary encoder.

## Features

- 4x4 Matrix Keypad
- Dual-mode Rotary Encoder:
  - Volume Control Mode (Up/Down/Mute)
  - Mouse Scroll Mode
- Mode Switching via Long Press
- LED Mode Indicator
- USB HID Support

## Hardware Requirements

- Raspberry Pi Pico (RP2040)
- 4x4 Matrix Keypad
- Rotary Encoder with Push Button
- LED Indicator
- Pull-up Resistors (if not using internal)

## Pin Configuration

```cpp
#define RT_BTN 16    // Rotary Encoder Button
#define ENC_A 14     // Rotary Encoder A
#define ENC_B 15     // Rotary Encoder B

// Keypad Matrix
#define ROW1 28
#define ROW2 27
#define ROW3 26
#define ROW4 22

#define COL1 0
#define COL2 1
#define COL3 2
#define COL4 3