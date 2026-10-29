#include "game.h"

#include <Arduino.h>
#include <esp_system.h>
#include <math.h>
#include "freertos/task.h"

#include "config.h"
#include "game_types.h"

namespace {
// Private to this translation unit. The render task receives copies via queue.
GameState game{};

void resetBall() {
    const float speed = static_cast<float>(
        SERVE_MIN_SPEED + esp_random() % SERVE_SPEED_VARIATIONS);
    constexpr float degToRad = 0.0174532925f;
    const float angle =
        (SERVE_MIN_ANGLE + esp_random() % SERVE_ANGLE_VARIATIONS) * degToRad;
    const float directionY = (esp_random() & 1U) ? 1.0f : -1.0f;

    game.ballX = SCREEN_W / 2.0f;
    game.ballY = SCREEN_H / 2.0f;
    // Preserve the existing serve: always towards the human on the left.
    game.ballVX = -speed * cosf(angle);
    game.ballVY = directionY * speed * sinf(angle);
}

void updatePlayer(const InputData &input) {
    const float targetY = PLAYER_MIN_Y +
        (input.joystickY / ADC_MAX_VALUE) * (PLAYER_MAX_Y - PLAYER_MIN_Y);
    game.paddleY = (1.0f - PLAYER_SMOOTHING) * game.paddleY +
        PLAYER_SMOOTHING * targetY;
}

void updateAI(float dt) {
    const float target = game.ballVX > 0 ? game.ballY : SCREEN_H / 2.0f;
    const float error = target - game.aiPaddleY;
    const float maxStep = AI_SPEED * dt;
    if (fabsf(error) > AI_DEADZONE) {
        game.aiPaddleY += fmaxf(-maxStep, fminf(maxStep, error));
    }
    const float halfPaddle = PADDLE_H / 2.0f;
    game.aiPaddleY = fmaxf(halfPaddle,
        fminf(SCREEN_H - halfPaddle, game.aiPaddleY));
}

void updateBall(float dt) {
    game.ballX += game.ballVX * dt;
    game.ballY += game.ballVY * dt;

    if (game.ballY <= BALL_R) {
        game.ballY = BALL_R;
        game.ballVY = fabsf(game.ballVY);
    }
    if (game.ballY >= SCREEN_H - 1 - BALL_R) {
        game.ballY = SCREEN_H - 1 - BALL_R;
        game.ballVY = -fabsf(game.ballVY);
    }

    const float playerTop = game.paddleY - PADDLE_H / 2.0f;
    const float playerBottom = game.paddleY + PADDLE_H / 2.0f;
    if (game.ballVX < 0 &&
        game.ballX - BALL_R <= PADDLE_X + PADDLE_W &&
        game.ballX + BALL_R >= PADDLE_X &&
        game.ballY + BALL_R >= playerTop &&
        game.ballY - BALL_R <= playerBottom) {
        game.ballX = PADDLE_X + PADDLE_W + BALL_R;
        game.ballVX = fabsf(game.ballVX);
    }

    const float aiTop = game.aiPaddleY - PADDLE_H / 2.0f;
    const float aiBottom = game.aiPaddleY + PADDLE_H / 2.0f;
    if (game.ballVX > 0 &&
        game.ballX + BALL_R >= AI_PADDLE_X &&
        game.ballX - BALL_R <= AI_PADDLE_X + PADDLE_W &&
        game.ballY + BALL_R >= aiTop &&
        game.ballY - BALL_R <= aiBottom) {
        game.ballX = AI_PADDLE_X - BALL_R;
        game.ballVX = -fabsf(game.ballVX);
    }

    if (game.ballX > SCREEN_W + BALL_R) {
        ++game.playerScore;
        resetBall();
        Serial.println("Player scores!");
        digitalWrite(LED_PIN, HIGH);
    }
    if (game.ballX < -BALL_R) {
        ++game.aiScore;
        resetBall();
        Serial.println("AI scores!");
        digitalWrite(LED_PIN, HIGH);
    }
}
}  // namespace

void gameBegin() {
    game = GameState{};
    game.paddleY = SCREEN_H / 2.0f;
    game.aiPaddleY = SCREEN_H / 2.0f;
    resetBall();
}

void gameTask(void *parameter) {
    const auto *context = static_cast<const GameTaskContext *>(parameter);
    InputData input{};
    const TickType_t period = pdMS_TO_TICKS(GAME_PERIOD_MS);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, period);
        if (xQueueReceive(context->inputQueue, &input, 0) == pdTRUE) {
            updatePlayer(input);
        }
        updateAI(GAME_DT_SECONDS);
        updateBall(GAME_DT_SECONDS);
        xQueueOverwrite(context->gameQueue, &game);
    }
}
