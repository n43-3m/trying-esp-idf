// Event Group – Error Broadcasting:
// Create an event group. Define one bit: SYSTEM_ERROR_BIT.
// Create three “listener” tasks. Each listener task waits (xEventGroupWaitBits) for SYSTEM_ERROR_BIT to be set (use xWaitForAllBits = pdFALSE and xClearOnExit = pdFALSE). When the bit is set, the listener task should print an error message specific to itself (e.g., “Listener A detected error!”) and perhaps enter a safe state (simulated by a loop with delay).
// Create one “monitor” task that simulates detecting an error after a delay (e.g., 5 seconds). When it detects the error, it sets SYSTEM_ERROR_BIT using xEventGroupSetBits.
// Observe how all three listener tasks unblock and react when the single error bit is set.

#include <cstdio>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define SYSTEM_ERROR_BIT (1<<0)

EventGroupHandle_t systemErrorEvent = NULL;
const char *TAG = "EVENT GROUP";

void task_listener(void *params){
  char taskID = *(char*)params;
  ESP_LOGI(TAG,"listener %c started",taskID);
  
  EventBits_t bits = xEventGroupWaitBits(systemErrorEvent,SYSTEM_ERROR_BIT,pdFALSE,pdFALSE,portMAX_DELAY);
  if((bits & SYSTEM_ERROR_BIT) == SYSTEM_ERROR_BIT){
    ESP_LOGE(TAG,"listener %c detected error!",taskID);
    for (int i = 0; i < 3; i++)
    {
      ESP_LOGI(TAG,"listener %c entering safe state for 1 sec.",taskID);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
  } 
  ESP_LOGI(TAG,"deleting listener %c",taskID);
  vTaskDelete(NULL);
}

void task_monitor(void *params){
  ESP_LOGI(TAG,"monitor task started");
  vTaskDelay(pdMS_TO_TICKS(5000));
  xEventGroupSetBits(systemErrorEvent,SYSTEM_ERROR_BIT);
  ESP_LOGI(TAG,"monitor task deleting");
  vTaskDelete(NULL);

}

extern "C" void app_main(void){
    ESP_LOGI(TAG,"main task started.");
    systemErrorEvent = xEventGroupCreate();
    if(systemErrorEvent == NULL){
      ESP_LOGE(TAG,"failed to create event group");
      return;
    }
    xTaskCreate(task_monitor,"monitor task",2048,NULL,5,NULL);
    char taskID[3] = {'A','B','C'};
    for (int i = 0; i < 3; i++)
    {
      char id[15];
      snprintf(id,sizeof(id),"listener%c",taskID[1]);
      xTaskCreate(task_listener,id,2048,&taskID[i],5,NULL);
      
    }

}