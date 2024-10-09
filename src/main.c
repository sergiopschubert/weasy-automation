// main.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "wifi_manager.h"
#include "led_task.h"
#include "state_machine.h"
#include <stdint.h>

void init_system(void) {
   // Inicializar o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Iniciar a task Wi-Fi
    wifi_start_task();   
    while (!is_wifi_connected()){
        
    }
    led_task_init();
}

void app_main(void) {
    init_system();
    state_machine_run();
}
