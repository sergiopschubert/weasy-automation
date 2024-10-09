// led_task.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_task.h"
#include "rom/gpio.h"


#define LED_PIN GPIO_NUM_2 // Definir o pino do LED (geralmente GPIO2 na ESP32)

static int blink_frequency_ms = 1000; 

void led_blink_task(void *pvParameter) {
    // Configurar o pino como saída
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        // Liga o LED
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(blink_frequency_ms / portTICK_PERIOD_MS);  // Usa a frequência atual

        // Desliga o LED
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(blink_frequency_ms / portTICK_PERIOD_MS);  // Usa a frequência atual
    }
}

void led_task_init(void) {
    // Criar a task que pisca o LED
    xTaskCreate(led_blink_task, "led_blink_task", 1024, NULL, 4, NULL);
}

// Função para alterar a frequência de piscamento
void set_led_blink_frequency(int frequency_ms) {
    blink_frequency_ms = frequency_ms;
}