#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "time_manager.h"
#include <time.h>
#include <sys/time.h>


static const char *TAG = "TIME_MANAGER";
TaskHandle_t xHandleScheduledTask = NULL;
TaskHandle_t xHandleCalibrateTask = NULL;

static bool time_synced = false;
bool flag_water_pump = false;
bool flag_init_time = false;
bool flag_calibrate = false;

// horário programado para calibração do RTC
scheduled_time_t scheduled_calibrate[1] = {
    {17,15,00}
};

// Lista de horários programados
scheduled_time_t scheduled_times[MAX_SCHEDULED_TIMES] = {
    {6,00,00},    // 06:00:00
    {10,00,00},  
    {12,30,00},  
    {16,00,00},   
    {18,30,00},  
    {21,00,00},   
    // {21, 15, 0}   
};


// Callback chamado quando a hora é sincronizada
void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Hora sincronizada");
    time_synced = true;
}

// Função para inicializar o SNTP e configurar o fuso horário
void initialize_time(void) { 
    ESP_LOGI(TAG, "Inicializando SNTP");

    if(flag_init_time == true){
        esp_sntp_stop();
    }

    // Inicializar o SNTP
    // Configurar o fuso horário da região
    setenv("TZ", "BRT3", 1); // Exemplo: Horário de Brasília
    tzset();
    
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
    flag_init_time = true;
    flag_calibrate = false;
    
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

    if (timeinfo.tm_year < (2024 - 1900)) {
        ESP_LOGW(TAG, "Hora não sincronizada, usando hora local");
    }

    // Formatar a hora como string
    strftime(time_str, max_len, "%c", &timeinfo);

}


// Função para encontrar o próximo horário programado
time_t get_next_scheduled_time(void) {
    time_t now;
    struct tm timeinfo_now;
    time(&now);
    localtime_r(&now, &timeinfo_now);

    time_t min_diff = 0;
    time_t next_time = 0;

    for (int i = 0; i < MAX_SCHEDULED_TIMES; i++) {
        struct tm scheduled_time_tm = timeinfo_now;
        scheduled_time_tm.tm_hour = scheduled_times[i].hour;
        scheduled_time_tm.tm_min = scheduled_times[i].minute;
        scheduled_time_tm.tm_sec = scheduled_times[i].second;

        time_t scheduled_time = mktime(&scheduled_time_tm);

        // Se o horário programado já passou hoje, ajuste para o próximo dia
        if (difftime(scheduled_time, now) < 0) {
            scheduled_time_tm.tm_mday += 1;
            scheduled_time = mktime(&scheduled_time_tm);
        }

        time_t diff = difftime(scheduled_time, now);

        if (min_diff == 0 || diff < min_diff) {
            min_diff = diff;
            next_time = scheduled_time;
        }
    }

    return next_time;
}

void scheduled_task(void *pvParameter) {
    while (1) {
        time_t next_time = get_next_scheduled_time();
        time_t now;
        time(&now);

        time_t delay_seconds = difftime(next_time, now);
        ESP_LOGI(TAG, "Próximo evento em %lld segundos", delay_seconds);

        if (delay_seconds > 0) {
            // Aguarda até o próximo horário
            vTaskDelay(delay_seconds * 1000 / portTICK_PERIOD_MS);
        }


        
        flag_water_pump = true;
        ESP_LOGI(TAG, "Water Pump: %d", flag_water_pump);
        
        // Aguarda 5 segundos antes de procurar o próximo horário
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void calibrate_time_task(void *pvParameter) {
    time_t now;
    struct tm timeinfo_now;
    time(&now);
    localtime_r(&now, &timeinfo_now);

    struct tm scheduled_time_tm = timeinfo_now;
    scheduled_time_tm.tm_hour = scheduled_calibrate[0].hour;
    scheduled_time_tm.tm_min = scheduled_calibrate[0].minute;
    scheduled_time_tm.tm_sec = scheduled_calibrate[0].second;

    time_t next_time = mktime(&scheduled_time_tm);

    if (difftime(next_time, now) < 0) {
        scheduled_time_tm.tm_mday += 1;
        next_time = mktime(&scheduled_time_tm);
    }

    while (1) {

        time_t now;
        time(&now);
        time_t delay_seconds = difftime(next_time, now);
        ESP_LOGI(TAG, "Próxima calibração em %lld segundos", delay_seconds);

        if (delay_seconds > 0) {
            // Aguarda até o próximo horário
            vTaskDelay(delay_seconds * 1000 / portTICK_PERIOD_MS);
        }
        
        
        flag_calibrate = true;
        ESP_LOGI(TAG, "Flag calibrate: %d", flag_calibrate );
        
        // Aguarda 10 segundos antes de procurar o próximo horário
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        scheduled_time_tm.tm_mday += 1;
        next_time = mktime(&scheduled_time_tm);
    }
}

void scheduled_task_init() {
    if(xHandleScheduledTask == NULL)
        xTaskCreate(scheduled_task, "scheduled_task", 4096, NULL, 5, &xHandleScheduledTask);
        
    eTaskState state = eTaskGetState(xHandleScheduledTask);

    switch(state) {
        case eSuspended:
            ESP_LOGI(TAG, "A task schedule estava suspensa.");
            vTaskResume(xHandleScheduledTask);
            ESP_LOGI(TAG, "A task schedule foi retomada.");
            break;
        default:
            break;
    }
}

void calibrate_task_init() {
    if(xHandleCalibrateTask == NULL){
        xTaskCreate(calibrate_time_task, "calibrate_time_task", 2048, NULL, 2, &xHandleCalibrateTask);
        ESP_LOGI(TAG, "A task calibrate foi criada.");
    }

    eTaskState state = eTaskGetState(xHandleCalibrateTask);

    switch(state) {
        case eSuspended:
            ESP_LOGI(TAG, "A task calibrate estava suspensa.");
            vTaskResume(xHandleCalibrateTask);
            ESP_LOGI(TAG, "A task calibrate foi retomada.");
            break;
        default:
            break;
    }
}