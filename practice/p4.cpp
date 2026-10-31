
// hared Resource Logger:
// Create a global structure holding several data fields (e.g., int id, float value, char status[10]).
// Create two tasks:
// Task Writer: Periodically (e.g., every second) modifies all fields of the global structure with new data. It must protect access using a mutex.
// Task Reader: Periodically (e.g., every 750ms) reads all fields from the global structure and prints them to the console. It must also use the same mutex to ensure it reads a consistent state (not halfway through an update by Task Writer).
// Observe the output to verify that the Reader task always prints consistent data sets, even though the Writer is modifying them concurrently.


#include <cstdio>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

const char *TAG = "main";
struct SharedData
{
  int id=0;
  float value=0.0;
  char status[10]="";
};
SharedData sharedData;
SemaphoreHandle_t sharedDataMutex = nullptr;


void writerTask(void *params){
  ESP_LOGI(TAG,"writer task started.");
  
  while (true)
  {
    if(xSemaphoreTake(sharedDataMutex,portMAX_DELAY) == pdTRUE){
      sharedData.id++;
      sharedData.value = sharedData.value+0.3f;
      strcpy(sharedData.status,(sharedData.id %2 ==0) ? "positive": "negative");
      // sharedData.status = ; 
      if(xSemaphoreGive(sharedDataMutex)!=pdTRUE){
        ESP_LOGE(TAG,"could give mutex for shared data. deleted ?");
      }
    }else{
      ESP_LOGE(TAG,"could not take mutex for shared data. deleted maybe?");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  
}
void readerTask(void *params){
  ESP_LOGI(TAG,"writer task started.");
  
  while (1)
  {
    if(xSemaphoreTake(sharedDataMutex,portMAX_DELAY) == pdTRUE){
      ESP_LOGI(TAG,"id: %d, value: %f, status: %s", sharedData.id,sharedData.value,sharedData.status);
      if(xSemaphoreGive(sharedDataMutex)!=pdTRUE){
        ESP_LOGE(TAG,"could give mutex for shared data. deleted ?");
      }
    }else{
      ESP_LOGE(TAG,"could not take mutex for shared data. deleted maybe?");
    }
    vTaskDelay(pdMS_TO_TICKS(750));
    
  }
  
  
}
extern "C" void app_main(void){
  ESP_LOGI(TAG,"main task started.");
  printf("sharedData:\n");
  printf("id: %d, float: %f, char: %s",sharedData.id,sharedData.value,sharedData.status);
  sharedDataMutex = xSemaphoreCreateMutex();
  if(sharedDataMutex==NULL) {
    ESP_LOGE(TAG,"could not create shared mutex");
    return ;
  }
  
  if(xTaskCreate(writerTask,"writerTask",2048,NULL,5,NULL)==pdFALSE){
    ESP_LOGE(TAG,"could not create writer task");
  }
  if(xTaskCreate(readerTask,"readerTask",2048,NULL,5,NULL)==pdFALSE){
    ESP_LOGE(TAG,"could not create reader task");
  }
  
}