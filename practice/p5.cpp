// Event Group – Multi-Sensor Ready:
// Create an event group.
// Define three bits: SENSOR_A_READY, SENSOR_B_READY, SENSOR_C_READY.
// Create three separate tasks, each simulating the initialization of one sensor (A, B, C) with different delays (e.g., 1s, 2.5s, 1.8s). When a sensor task finishes init, it sets its corresponding bit in the event group.
// Create a fourth task that waits (xEventGroupWaitBits) for all three sensor bits to be set before printing “All sensors initialized and ready!”.

#include <cstdio>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define SENSOR_A_READY (1<<0)
#define SENSOR_B_READY (1<<1)
#define SENSOR_C_READY (1<<2)
#define SYSTEM_READY (SENSOR_A_READY|SENSOR_B_READY|SENSOR_C_READY)

const char *TAG = "EVENT_GROUP";

EventGroupHandle_t systemInitEventHandle = NULL;


void task_init_sensor_a(void *params){
  ESP_LOGI(TAG,"init sensor a");
  vTaskDelay(pdMS_TO_TICKS(1000));
  EventBits_t bits = xEventGroupSetBits(systemInitEventHandle,SENSOR_A_READY);
  ESP_LOGI(TAG,"init sensor a completed. bits: %d",bits);
  vTaskDelete(NULL);
}
void task_init_sensor_b(void *params){
  ESP_LOGI(TAG,"init sensor b");
  vTaskDelay(pdMS_TO_TICKS(10000));
  EventBits_t bits = xEventGroupSetBits(systemInitEventHandle,SENSOR_B_READY);
  ESP_LOGI(TAG,"init sensor b completed.");
  vTaskDelete(NULL);
}
void task_init_sensor_c(void *params){
  ESP_LOGI(TAG,"init sensor c");
  vTaskDelay(pdMS_TO_TICKS(5000));
  EventBits_t bits = xEventGroupSetBits(systemInitEventHandle,SENSOR_C_READY);
  ESP_LOGI(TAG,"init sensor c completed.");
  vTaskDelete(NULL);
}
void init_sensors(void *params){
  ESP_LOGI(TAG,"init sensors...");
  EventBits_t bits = xEventGroupWaitBits(systemInitEventHandle,SYSTEM_READY,pdFALSE,pdTRUE,portMAX_DELAY);
  if((bits & SYSTEM_READY)==SYSTEM_READY){
    ESP_LOGI(TAG,"all sensors started successfully");
    
  }else{
    ESP_LOGE(TAG,"sensor init failed."); // this should be impossible when portMax_DELAY is selected.
    
  }
  ESP_LOGI(TAG,"cleaning up...");
  vEventGroupDelete(systemInitEventHandle);
  vTaskDelete(NULL);

}
extern "C" void app_main(void){
  systemInitEventHandle = xEventGroupCreate();

  if(systemInitEventHandle == NULL){
    ESP_LOGE(TAG, "Failed to create event group.");
    return; // Handle error
  }
  ESP_LOGI(TAG, "System event group created.");
  xTaskCreate(init_sensors, "sensorInit", 2048, NULL, 5, NULL);
  xTaskCreate(task_init_sensor_a, "sensorInitA", 2048, NULL, 5, NULL);
  xTaskCreate(task_init_sensor_b, "sensorIniB", 2048, NULL, 5, NULL);
  xTaskCreate(task_init_sensor_c, "sensorInitC", 2048, NULL, 5, NULL);
  ESP_LOGI(TAG, "App Main: Tasks created.");  
}