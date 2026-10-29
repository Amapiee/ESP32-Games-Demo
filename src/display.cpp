#include "display.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "freertos/task.h"

#include "config.h"
#include "game_types.h"

namespace {
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

void drawGame(const GameState &state) {
    display.clearDisplay();
    display.fillRect(
        PADDLE_X,
        static_cast<int>(state.paddleY - PADDLE_H / 2.0f),
        PADDLE_W, PADDLE_H, SSD1306_WHITE);
    display.fillRect(
        AI_PADDLE_X,
        static_cast<int>(state.aiPaddleY - PADDLE_H / 2.0f),
        PADDLE_W, PADDLE_H, SSD1306_WHITE);

    display.setCursor(34, 0);
    display.print(state.playerScore);
    display.print(" : ");
    display.print(state.aiScore);
    display.fillCircle(
        static_cast<int>(state.ballX),
        static_cast<int>(state.ballY), BALL_R, SSD1306_WHITE);
    display.display();
}
}  // namespace

bool displayBegin() {
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        return false;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("ESP-PONG starting...");
    display.display();
    return true;
}

void renderTask(void *parameter) {
    const auto *context = static_cast<const DisplayTaskContext *>(parameter);
    GameState state{};
    for (;;) {
        if (xQueueReceive(context->gameQueue, &state,
                          pdMS_TO_TICKS(RENDER_WAIT_MS)) == pdTRUE) {
            drawGame(state);
        }
        vTaskDelay(pdMS_TO_TICKS(RENDER_DELAY_MS));
    }
}
