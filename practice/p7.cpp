// Deferred Processing with Task Notifications: Modify Example 2 to use FreeRTOS Task Notifications instead of a semaphore. The ISR should use vTaskNotifyGiveFromISR(), and the handler task should wait using ulTaskNotifyTake().

#include <cstdio>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"

#define BUTTON_GPIO GPIO_NUM_5
// Global semaphore handle
// SemaphoreHandle_t gpio_sem = NULL;
const char *TAG = "system";
TaskHandle_t gpio_handler_task_handle = NULL;
// Task function to handle the deferred processing
static void gpio_handler_task(void* arg) {
    uint32_t gpio_num = (uint32_t) arg; // Could pass specific data if needed
    while(1) {
        uint32_t notif = ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        ESP_LOGI(TAG, "Handler Task: notif %lu ", notif);
        if(notif>0){
          ESP_LOGI(TAG, "Handler Task: GPIO %lu interrupt processed!", gpio_num);
        }
    }
}

// ISR handler function - kept minimal
static void IRAM_ATTR gpio_isr_handler_defer(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(
    gpio_handler_task_handle,
    &xHigherPriorityTaskWoken
);

    // Check if we need to yield
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting Deferred ISR (Semaphore) Example...");

    // Create the binary semaphore
    // Must be created BEFORE the ISR is registered
    // gpio_sem = xSemaphoreCreateBinary();
    // if (gpio_sem == NULL) {
    //     ESP_LOGE(TAG, "Failed to create semaphore");
    //     return;
    // }

    // Configure the GPIO pin (same as Example 1)
    gpio_config_t io_conf;
    memset(&io_conf, 0, sizeof(io_conf));
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Install the global GPIO ISR service
    esp_err_t install_err = gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
     if (install_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install GPIO ISR service: %s", esp_err_to_name(install_err));
        // vSemaphoreDelete(gpio_sem); // Clean up semaphore
        return;
    }

    // Add the specific handler for our button GPIO
    esp_err_t add_err = gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler_defer, NULL); // No arg needed here
     if (add_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add ISR handler for GPIO %d: %s", BUTTON_GPIO, esp_err_to_name(add_err));
        gpio_uninstall_isr_service();
        // vSemaphoreDelete(gpio_sem);
        return;
    }

    // Create the handler task
    // Pass the GPIO number as argument just for logging purposes in the task
    xTaskCreate(
    gpio_handler_task,
    "gpio_handler_task",
    2048,
    (void*)BUTTON_GPIO,
    10,
    &gpio_handler_task_handle
);

    ESP_LOGI(TAG, "ISR handler and task created for GPIO %d. Press the button...", BUTTON_GPIO);

    // Main task can now do other things or just idle
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
