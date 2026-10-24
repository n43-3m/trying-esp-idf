// Core Load Simulation (Dual-Core):
// On an ESP32/S3, create two tasks performing floating-point calculations in a tight loop (e.g., volatile double x = 1.23; while(1) { x = sin(x) * cos(x); }).
// Task A: Pin to Core 0, Priority 5.
// Task B: Pin to Core 1, Priority 5.
// Create a third, low-priority task (Priority 1, no affinity) that simply prints “Low priority task running” every 2 seconds using vTaskDelay.
// Observe if the low-priority task still gets scheduled and prints its message, demonstrating that even with both cores busy, the RTOS scheduler allows lower-priority tasks some time (due to tick interrupts/scheduler calls).

#include <cstdio>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void calculateTask(void *params){
    volatile double x = 1.23;

    while (true)
    {
        
        // x = sin(x) * cos(x); // this solution will cause a watchdog timeout and crash as other tasks cant get cpu time to do systems tasks



        // Perform a batch of calculations.
        for (int i = 0; i < 10000; ++i)
        {
            x = sin(x) * cos(x);
        }
        // Give other tasks and the idle task CPU time.
        vTaskDelay(1);
    }
    
}
void printTask(void *params){
    while (true)
    {
        ESP_LOGI("print","low priority task running");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
}

extern "C" void app_main(void){
    if(xTaskCreatePinnedToCore(
        calculateTask,
        "calculate_core0",
        2048,
        NULL,
        5,
        NULL,
        0
    )!=pdPASS){
        ESP_LOGE("main","failed to create calculate task on core0");
    }
ESP_LOGI("main","created calculate task on core0");
    if(xTaskCreatePinnedToCore(
        calculateTask,
        "calculate_core1",
        2048,
        NULL,
        5,
        NULL,
        1
    )!=pdPASS){
        ESP_LOGE("main","failed to create calculate task on core1");
    }
    ESP_LOGI("main","created calculate task on core1");
    if(xTaskCreatePinnedToCore(
        printTask,
        "printTask",
        2048,
        NULL,
        3,
        NULL,
        tskNO_AFFINITY
    )!=pdPASS){
        ESP_LOGE("main","failed to create print task ");
    }
    ESP_LOGI("main","created calculate task");
}