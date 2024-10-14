#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "wifi_manager.h"
#include "credentials.h"

static const char *TAG = "WIFI_MANAGER";
bool wifi_connected = NULL;


// bool should_send_log(const char *tag);

static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            ESP_LOGI(TAG, "Wi-Fi iniciado, tentando conectar...");
            esp_wifi_connect();
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            wifi_connected = false;
            ESP_LOGW(TAG, "Wi-Fi desconectado.");
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi_connected = true;
        ESP_LOGI(TAG, "Wi-Fi conectado, endereço IP atribuído.");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Endereço IP: " IPSTR, IP2STR(&event->ip_info.ip));
        // process_logs_in_nvs();
    }
}


// Função para inicializar o Wi-Fi
void wifi_init_sta(void) {
    
    esp_netif_init(); // Inicializar o TCP/IP stack  
    esp_event_loop_create_default(); // Criar o loop de eventos padrão
    esp_netif_create_default_wifi_sta();// Criar um driver de interface Wi-Fi
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

    // Definir o modo Wi-Fi como Station
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);

    // Iniciar o Wi-Fi
    esp_wifi_start();

    ESP_LOGI(TAG, "Wi-Fi iniciado, conectando-se ao SSID: %s", WIFI_SSID);
}

// Task que controla o Wi-Fi
void wifi_task(void *pvParameters) {
    wifi_init_sta();
    while (true) {
        if(is_wifi_connected()){
            ESP_LOGI(TAG, "Wi-Fi Task rodando...");
            vTaskDelay(300000 / portTICK_PERIOD_MS);
        }
        else if(!is_wifi_connected()){
            ESP_LOGI(TAG, "Wi-Fi entrou na segunda condição...");
            esp_wifi_connect();
            vTaskDelay(30000 / portTICK_PERIOD_MS);
        }
    }
}

// Função para verificar o status da conexão Wi-Fi
bool is_wifi_connected(void) {
    return wifi_connected;
}

// Função para criar a task de Wi-Fi
void wifi_start_task(void) {
    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 5, NULL);
}


