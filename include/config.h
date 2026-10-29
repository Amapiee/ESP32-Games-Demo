#pragma once

#include <stdint.h>

// Hardware: ESP32-WROOM / PlatformIO board esp32dev.
constexpr int OLED_SDA = 21;
constexpr int OLED_SCL = 22;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr int JOYSTICK_Y = 35;  // ADC1
constexpr int LED_PIN = 4;
constexpr int MODE_BUTTON_PIN = 27;  // Reserved; mode switching is not implemented.

constexpr int SCREEN_W = 128;
constexpr int SCREEN_H = 64;
constexpr int PADDLE_X = 4;
constexpr int PADDLE_W = 3;
constexpr int PADDLE_H = 14;
constexpr int BALL_R = 2;
constexpr int AI_PADDLE_X = SCREEN_W - PADDLE_X - PADDLE_W;

// Gameplay: the same values as the single-file AI demo.
constexpr float AI_SPEED = 32.0f;       
constexpr float AI_DEADZONE = 3.0f;     // Pixels; AI paddle does not move if the ball is within this distance.
constexpr int ADC_BITS = 12;            // ADC resolution in bits; ESP32 ADC1 is 12 bits.
constexpr float ADC_MAX_VALUE = 4095.0f;
constexpr float PLAYER_MIN_Y = 8.0f;
constexpr float PLAYER_MAX_Y = 56.0f;
constexpr float PLAYER_SMOOTHING = 0.3f;
constexpr uint32_t SERVE_MIN_SPEED = 50;
constexpr uint32_t SERVE_SPEED_VARIATIONS = 20;  // 50..69 pixels/second
constexpr uint32_t SERVE_MIN_ANGLE = 20;
constexpr uint32_t SERVE_ANGLE_VARIATIONS = 31;  // 20..50 degrees

// Timing. OLED transfer time is additional to RENDER_DELAY_MS.
constexpr uint32_t INPUT_PERIOD_MS = 10;
constexpr uint32_t GAME_PERIOD_MS = 16;
constexpr uint32_t RENDER_DELAY_MS = 33;
constexpr uint32_t RENDER_WAIT_MS = 100;
constexpr float GAME_DT_SECONDS = GAME_PERIOD_MS / 1000.0f;

// ESP-IDF task stack sizes are in bytes.
constexpr uint32_t INPUT_STACK_BYTES = 2048;
constexpr uint32_t GAME_STACK_BYTES = 3072;
constexpr uint32_t DISPLAY_STACK_BYTES = 4096;
constexpr int INPUT_PRIORITY = 2;
constexpr int GAME_PRIORITY = 3;
constexpr int DISPLAY_PRIORITY = 1;
constexpr int INPUT_CORE = 1;
constexpr int GAME_CORE = 1;
constexpr int DISPLAY_CORE = 0;
