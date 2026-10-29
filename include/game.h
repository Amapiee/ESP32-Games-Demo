#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct GameTaskContext {
    QueueHandle_t inputQueue;
    QueueHandle_t gameQueue;
};

// Call once, before starting the game task.
void gameBegin();
// Only this task modifies the game state after startup.
void gameTask(void *parameter);
