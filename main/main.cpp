#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"
#include "esp_log.h"
#include "stdio.h"
#include "driver/uart.h"

TaskHandle_t workerTaskHandle = nullptr;
TaskHandle_t monitorTaskHandle = nullptr;

// 2. Task function
void workerTask(void *params)
{
    int counter = 0;
    while (true)
    {   counter++;
        // std::cout << "Counter:" << counter << std::endl;
        ESP_LOGI("workerTask","Counter: %i",counter);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}


void monitorTask(void *params)
{
    char input;

    while (true)
    {
        if (workerTaskHandle == nullptr)
        {
            ESP_LOGI("monitor",
                     "Worker task deleted. Monitor exiting.");
            monitorTaskHandle = nullptr;
            vTaskDelete(NULL);
            return;
        }

        int len = uart_read_bytes(
            UART_NUM_0,
            (uint8_t *)&input,
            1,
            portMAX_DELAY
        );

        // Ignore Enter
        if (input == '\r' || input == '\n')
        {
            continue;
        }

        ESP_LOGI("monitor", "Input received: %c", input);

        switch (input)
        {
            case 's':
                vTaskSuspend(workerTaskHandle);
                ESP_LOGI("monitor", "Worker suspended");
                break;

            case 'r':
                vTaskResume(workerTaskHandle);
                ESP_LOGI("monitor", "Worker resumed");
                break;

            case 'p':
                vTaskPrioritySet(workerTaskHandle, 7);
                ESP_LOGI("monitor", "Worker priority set to 7");
                break;

            case 'd':
                ESP_LOGI("monitor", "Deleting worker task");
                vTaskDelete(workerTaskHandle);
                workerTaskHandle = nullptr;
                break;

            default:
                ESP_LOGW("monitor", "Invalid input: %c", input);
                break;
        }
    }
}


// 6. Main application entry point
extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(uart_driver_install(
    UART_NUM_0,
    2048,  // RX buffer size
    0,     // TX buffer size
    0,     // Event queue size
    NULL,
    0
));

    BaseType_t result = xTaskCreate(
        workerTask,
        "workerTask",
        2048,
        NULL,
        5,
        &workerTaskHandle
    );
    
    if(result !=pdPASS){
        ESP_LOGI("main","failed to create worker task");
    }
    

    BaseType_t monitorRes = xTaskCreate(
        monitorTask,
        "monitorTask",
        2048,
        NULL,
        5,
        &monitorTaskHandle
    );
    
    if(monitorRes !=pdPASS){
        ESP_LOGI("main","failed to create monitor task");
    }


}
