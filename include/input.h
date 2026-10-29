#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct InputTaskContext {
    QueueHandle_t inputQueue;
};

void inputBegin();
// parameter must point to an InputTaskContext that outlives the task.
void inputTask(void *parameter);
