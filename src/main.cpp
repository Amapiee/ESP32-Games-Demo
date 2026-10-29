#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "config.h"
#include "display.h"
#include "game.h"
#include "game_types.h"
#include "input.h"
#include "network.h"

namespace {
// Static storage: these contexts remain valid after setup() returns.
InputTaskContext inputContext{};
GameTaskContext gameContext{};
DisplayTaskContext displayContext{};

void stopWithError(const char *message) {
    Serial.println(message);
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
}  // namespace

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    inputBegin();
    if (!displayBegin()) {
        stopWithError("OLED initialization failed!");
    }
    networkBegin();  // Placeholder; this version plays against the AI.

    const QueueHandle_t inputQueue = xQueueCreate(1, sizeof(InputData));
    const QueueHandle_t gameQueue = xQueueCreate(1, sizeof(GameState));
    if (inputQueue == nullptr || gameQueue == nullptr) {
        stopWithError("Queue creation failed!");
    }

    inputContext.inputQueue = inputQueue;
    gameContext.inputQueue = inputQueue;
    gameContext.gameQueue = gameQueue;
    displayContext.gameQueue = gameQueue;
    gameBegin();

    TaskHandle_t inputHandle = nullptr;
    TaskHandle_t gameHandle = nullptr;
    TaskHandle_t displayHandle = nullptr;
    const bool created =
        xTaskCreatePinnedToCore(
            inputTask, "InputTask", INPUT_STACK_BYTES, &inputContext,
            INPUT_PRIORITY, &inputHandle, INPUT_CORE) == pdPASS &&
        xTaskCreatePinnedToCore(
            gameTask, "GameTask", GAME_STACK_BYTES, &gameContext,
            GAME_PRIORITY, &gameHandle, GAME_CORE) == pdPASS &&
        xTaskCreatePinnedToCore(
            renderTask, "RenderTask", DISPLAY_STACK_BYTES, &displayContext,
            DISPLAY_PRIORITY, &displayHandle, DISPLAY_CORE) == pdPASS;

    if (!created) {
        if (inputHandle != nullptr) vTaskDelete(inputHandle);
        if (gameHandle != nullptr) vTaskDelete(gameHandle);
        if (displayHandle != nullptr) vTaskDelete(displayHandle);
        stopWithError("Task creation failed!");
    }
    Serial.println("ESP-PONG tasks started. Mode: player vs AI.");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
