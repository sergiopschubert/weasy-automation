#include "pump_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "PUMP_CONTROL";

void configure_gpio(void) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_OUTPUT_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(GPIO_OUTPUT_PIN, 0);
}

void activate_gpio_for_duration(uint32_t duration_s) {
    gpio_set_level(GPIO_OUTPUT_PIN, 1);
    ESP_LOGI(TAG, "GPIO %d ativado", GPIO_OUTPUT_PIN);

    vTaskDelay(duration_s * 1000 / portTICK_PERIOD_MS);

    gpio_set_level(GPIO_OUTPUT_PIN, 0);
    ESP_LOGI(TAG, "GPIO %d desativado", GPIO_OUTPUT_PIN);
}
