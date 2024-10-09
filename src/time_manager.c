// time_manager.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "time_manager.h"
#include <time.h>
#include <sys/time.h>


static const char *TAG = "time_manager";

// Variável para verificar se a hora foi sincronizada
static bool time_synced = false;

// Callback chamado quando a hora é sincronizada
void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Hora sincronizada");
    time_synced = true;
}

// Função para inicializar o SNTP e configurar o fuso horário
void initialize_time(void) { 
    ESP_LOGI(TAG, "Inicializando SNTP");

    // Inicializar o SNTP
        // Configurar o fuso horário da região
    setenv("TZ", "EET-3EEST,M3.5.0/3,M10.5.0/4", 1);  // Exemplo: Horário de Brasília
    tzset();
    
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();


}

// Função para verificar se a hora foi sincronizada
bool is_time_synced(void) {
    return time_synced;
}

// Função para obter a hora atual formatada
void get_current_time(char *time_str, size_t max_len) {
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2020 - 1900)) {
        ESP_LOGW(TAG, "Hora não sincronizada, usando hora local");
    }

    // Formatar a hora como string
    strftime(time_str, max_len, "%c", &timeinfo);
}
