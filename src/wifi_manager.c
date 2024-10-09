// // wifi_manager.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "wifi_manager.h"

// Definir o nome da rede Wi-Fi e a senha
#define WIFI_SSID      "APARTAMENTO 233 - 2.4G"
#define WIFI_PASS      "FRANGOFRITO2"

static const char *TAG = "wifi_manager";
bool wifi_connected = false;

// Função para lidar com eventos de Wi-Fi
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Tentando reconectar ao Wi-Fi...");
        esp_wifi_connect();
        wifi_connected = false;  
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        wifi_connected = true;  
        ESP_LOGI(TAG, "Conectado com sucesso, IP obtido: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

// Função para inicializar o Wi-Fi
void wifi_init_sta(void) {
    // Inicializar o TCP/IP stack
    esp_netif_init();

    // Criar o loop de eventos padrão
    esp_event_loop_create_default();

    // Criar um driver de interface Wi-Fi
    esp_netif_create_default_wifi_sta();

    // Inicializar o driver Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Registrar eventos de Wi-Fi
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    // Configurações da rede Wi-Fi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    // Definir o modo Wi-Fi como Station (conectar a uma rede)
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);

    // Iniciar o Wi-Fi
    esp_wifi_start();

    ESP_LOGI(TAG, "Wi-Fi iniciado, conectando-se ao SSID: %s", WIFI_SSID);
}

// Task que controla o Wi-Fi
void wifi_task(void *pvParameters) {
    // Inicializar o Wi-Fi
    wifi_init_sta();

    while (true) {
        // Aqui você pode monitorar o status do Wi-Fi ou adicionar outras operações
        ESP_LOGI(TAG, "Wi-Fi Task rodando...");
        vTaskDelay(10000 / portTICK_PERIOD_MS); // Delay de 10 segundos
    }
}

// Função para criar a task de Wi-Fi
void wifi_start_task(void) {
    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 5, NULL);
}

// Função para verificar o status da conexão Wi-Fi
bool is_wifi_connected(void) {
    return wifi_connected;
}