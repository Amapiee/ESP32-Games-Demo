#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct DisplayTaskContext {
    QueueHandle_t gameQueue;
};

bool displayBegin();
// parameter must point to a DisplayTaskContext that outlives the task.
void renderTask(void *parameter);
