#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueButId;
QueueHandle_t xQueueButR;
QueueHandle_t xQueueButG;
SemaphoreHandle_t xSemaphore_g;
SemaphoreHandle_t xSemaphore_r;


void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) { // fall edge
        if (gpio == BTN_PIN_R) {
            xSemaphoreGiveFromISR(xSemaphore_r, 0);
            xQueueSendFromISR(xQueueButR, &gpio, 0);
        } else if (gpio == BTN_PIN_G) {
            xSemaphoreGiveFromISR(xSemaphore_g, 0);
            xQueueSendFromISR(xQueueButG, &gpio, 0);
        }
        
        
    }
}


// LEDS
void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;

    while (true) {
        if (xQueueReceive(xQueueButR, &delay, 0)) {
            printf("r: %d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButG, &delay, 0)) {
            printf("g: %d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}


// BTNS
void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    int delay_r = 0, delay_g = 0;
    int id = 0;
    while (true) {
        // xQueueReceive(xQueueButId, &id, 0);
        if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE
        // && id == BTN_PIN_R
        ) {
            if (delay_r < 1000) {
                delay_r += 100;
            } else {
                delay_r = 100;
            }
            printf("delay btn r %d \n", delay_r);
            xQueueSend(xQueueButR, &delay_r, 0);

        } else if (xSemaphoreTake(xSemaphore_g, pdMS_TO_TICKS(500)) == pdTRUE
        // && id == BTN_PIN_G
        ) {
            if (delay_g < 1000) {
                delay_g += 100;
            } else {
                delay_g = 100;
            }
            printf("delay btn g %d \n", delay_g);
            xQueueSend(xQueueButG, &delay_g, 0);
        }
    }
}


int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    // xQueueButId = xQueueCreate(32, sizeof(int));
    xQueueButR = xQueueCreate(32, sizeof(int));
    xQueueButG = xQueueCreate(32, sizeof(int));

    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_g = xSemaphoreCreateBinary();

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
