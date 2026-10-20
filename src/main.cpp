
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// ---------- Hardware configuration ----------
constexpr int OLED_SDA = 21;
constexpr int OLED_SCL = 22;
constexpr int JOYSTICK_Y = 35;  // ADC1

constexpr int SCREEN_W = 128;
constexpr int SCREEN_H = 64;

constexpr int PADDLE_X = 4;
constexpr int PADDLE_W = 3;
constexpr int PADDLE_H = 14;
constexpr int BALL_R = 2;

Adafruit_SSD1306 display(
    SCREEN_W, SCREEN_H, &Wire, -1
);

// ---------- Data exchanged between tasks ----------
struct InputData {
    int joystickY;
};

struct GameState {
    float paddleY;
    float ballX;
    float ballY;
    float ballVX;
    float ballVY;
};

// Queue length = 1: keep the latest data only.
QueueHandle_t inputQueue;
QueueHandle_t gameQueue;

// ---------- Initial game state ----------
GameState game = {
    32.0f,   // paddle center Y
    64.0f,   // ball X
    32.0f,   // ball Y
    -45.0f,  // ball velocity X, pixels/second
    25.0f    // ball velocity Y, pixels/second
};

// ---------- Task 1: Read joystick ----------
void inputTask(void *parameter) {
    InputData input{};

    for (;;) {
        input.joystickY = analogRead(JOYSTICK_Y);

        // Replace old input with the latest sample.
        xQueueOverwrite(inputQueue, &input);

        // Sample at approximately 100 Hz.
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ---------- Task 2: Update game physics ----------
void gameTask(void *parameter) {
    InputData input{};
    const TickType_t period = pdMS_TO_TICKS(16);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        // Fixed-period game update, approximately 60 Hz.
        vTaskDelayUntil(&lastWake, period);

        if (xQueueReceive(inputQueue, &input, 0) == pdTRUE) {
            // Map ADC reading to the valid paddle range.
            float targetY =
                8.0f + (input.joystickY / 4095.0f) * 48.0f;

            // Light smoothing to reduce jitter.
            game.paddleY =
                0.7f * game.paddleY + 0.3f * targetY;
        }

        // Approximate elapsed time per game update.
        constexpr float dt = 0.016f;

        game.ballX += game.ballVX * dt;
        game.ballY += game.ballVY * dt;

        // Bounce off top and bottom edges.
        if (game.ballY <= BALL_R) {
            game.ballY = BALL_R;
            game.ballVY = fabsf(game.ballVY);
        }

        if (game.ballY >= SCREEN_H - 1 - BALL_R) {
            game.ballY = SCREEN_H - 1 - BALL_R;
            game.ballVY = -fabsf(game.ballVY);
        }

        // Paddle collision.
        const float paddleTop = game.paddleY - PADDLE_H / 2.0f;
        const float paddleBottom = game.paddleY + PADDLE_H / 2.0f;

        if (game.ballVX < 0 &&
            game.ballX - BALL_R <= PADDLE_X + PADDLE_W &&
            game.ballX + BALL_R >= PADDLE_X &&
            game.ballY >= paddleTop &&
            game.ballY <= paddleBottom) {

            game.ballX = PADDLE_X + PADDLE_W + BALL_R;
            game.ballVX = fabsf(game.ballVX);
        }

        // If the ball leaves the right edge, restart it.
        if (game.ballX > SCREEN_W + BALL_R) {
            game.ballX = SCREEN_W / 2.0f;
            game.ballY = SCREEN_H / 2.0f;
            game.ballVX = -45.0f;
            game.ballVY = 25.0f;
        }

        // If the player misses the ball, restart it.
        if (game.ballX < -BALL_R) {
            game.ballX = SCREEN_W / 2.0f;
            game.ballY = SCREEN_H / 2.0f;
            game.ballVX = -45.0f;
            game.ballVY = 25.0f;
        }

        // Send a snapshot to the rendering task.
        xQueueOverwrite(gameQueue, &game);
    }
}

// ---------- Task 3: Render OLED ----------
void renderTask(void *parameter) {
    GameState state{};

    for (;;) {
        if (xQueueReceive(gameQueue, &state,
                          pdMS_TO_TICKS(100)) == pdTRUE) {

            display.clearDisplay();

            // Draw the player's paddle.
            int paddleTop =
                static_cast<int>(state.paddleY - PADDLE_H / 2.0f);

            display.fillRect(
                PADDLE_X, paddleTop,
                PADDLE_W, PADDLE_H,
                SSD1306_WHITE
            );

            // Draw the ball.
            display.fillCircle(
                static_cast<int>(state.ballX),
                static_cast<int>(state.ballY),
                BALL_R,
                SSD1306_WHITE
            );

            display.display();
        }

        // Limit OLED refresh rate to around 30 Hz.
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

void setup() {
    Serial.begin(115200);

    analogReadResolution(12);

    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(
            SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED initialization failed!");
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("ESP-PONG starting...");
    display.display();

    // Create queues before starting the tasks.
    inputQueue = xQueueCreate(1, sizeof(InputData));
    gameQueue = xQueueCreate(1, sizeof(GameState));

    if (inputQueue == nullptr || gameQueue == nullptr) {
        Serial.println("Queue creation failed!");
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Core 1: time-sensitive input and game update.
    xTaskCreatePinnedToCore(
        inputTask, "InputTask",
        2048, nullptr, 2, nullptr, 1
    );

    xTaskCreatePinnedToCore(
        gameTask, "GameTask",
        3072, nullptr, 3, nullptr, 1
    );

    // Core 0: OLED rendering.
    xTaskCreatePinnedToCore(
        renderTask, "RenderTask",
        4096, nullptr, 1, nullptr, 0
    );

    Serial.println("ESP-PONG tasks started.");
}

void loop() {
    // Application work is handled by FreeRTOS tasks.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
