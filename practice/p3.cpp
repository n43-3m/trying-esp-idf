
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// 1. Define command types
enum CommandType {
    CMD_LED_ON,
    CMD_LED_OFF,
    CMD_PRINT_MSG
};

// 2. Define the command structure
struct Command {
    CommandType type;
    char message[32];
};

// Global command queue
QueueHandle_t commandQueue = NULL;

// 3. Command Sender task
void commandSenderTask(void *pvParameters)
{
    Command commands[] = {
        {CMD_LED_ON, ""},
        {CMD_PRINT_MSG, "Hello"},
        {CMD_LED_OFF, ""},
        {CMD_PRINT_MSG, "World"}
    };

    const int commandCount =
        sizeof(commands) / sizeof(commands[0]);

    int index = 0;

    while (true) {
        // Send the next command to the queue
        if (xQueueSend(commandQueue, &commands[index],
                       pdMS_TO_TICKS(1000)) == pdPASS) {
            printf("Sender: Command sent (type = %d)\n",
                   commands[index].type);
        } else {
            printf("Sender: Queue full, command not sent\n");
        }

        // Move to the next command
        index = (index + 1) % commandCount;

        // Wait before sending the next command
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// 4. Command Processor task
void commandProcessorTask(void *pvParameters)
{
    Command receivedCommand;

    while (true) {
        // Block until a command is received
        if (xQueueReceive(commandQueue, &receivedCommand,
                          portMAX_DELAY) == pdPASS) {

            // Process the received command
            switch (receivedCommand.type) {

                case CMD_LED_ON:
                    printf("Processor: LED ON\n");
                    break;

                case CMD_LED_OFF:
                    printf("Processor: LED OFF\n");
                    break;

                case CMD_PRINT_MSG:
                    printf("Processor: %s\n",
                           receivedCommand.message);
                    break;

                default:
                    printf("Processor: Unknown command\n");
                    break;
            }
        }
    }
}

// 5. ESP-IDF application entry point
extern "C" void app_main(void)
{
    // Create queue with length 5
    commandQueue = xQueueCreate(5, sizeof(Command));

    if (commandQueue == NULL) {
        printf("Error: Failed to create command queue\n");
        return;
    }

    // Create Command Sender task
    xTaskCreate(
        commandSenderTask,
        "CommandSender",
        2048,
        NULL,
        5,
        NULL
    );

    // Create Command Processor task
    xTaskCreate(
        commandProcessorTask,
        "CommandProcessor",
        2048,
        NULL,
        5,
        NULL
    );

    printf("Command system started\n");
}
