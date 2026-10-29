#include "input.h"

#include <Arduino.h>
#include "freertos/task.h"

#include "config.h"
#include "game_types.h"

void inputBegin() {
    analogReadResolution(ADC_BITS);
    pinMode(JOYSTICK_Y, INPUT);
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
}

void inputTask(void *parameter) {
    const auto *context = static_cast<const InputTaskContext *>(parameter);
    InputData input{};

    for (;;) {
        input.joystickY = analogRead(JOYSTICK_Y);
        // Queue length is one: retain the newest sample.
        xQueueOverwrite(context->inputQueue, &input);
        vTaskDelay(pdMS_TO_TICKS(INPUT_PERIOD_MS));
    }
}
