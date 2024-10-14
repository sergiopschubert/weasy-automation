#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_task.h"
#include "state_machine.h"
#include "time_manager.h"
#include "esp_log.h"
#include <time.h>
#include "pump_control.h"
#include "wifi_manager.h"
#include "firebase_manager.h"

static const char *TAG = "STATE_MACHINE";
static int pump_time = 30; 


// Variável que armazena o estado atual
static state_t current_state = STATE_INIT;

// Definição das funções de estado
void state_init(void);
void state_get_time(void);


// Implementação da função para o estado inicial
void state_init(void) {
    vTaskDelay(200 / portTICK_PERIOD_MS); 
    configure_gpio();

    vTaskDelay(200 / portTICK_PERIOD_MS); 
    wifi_start_task();  // Iniciar a task Wi-Fi

    vTaskDelay(200 / portTICK_PERIOD_MS); 
    while (!is_wifi_connected())

    vTaskDelay(200 / portTICK_PERIOD_MS); 
    led_task_init();
    set_led_blink_frequency(1000);

    vTaskDelay(100 / portTICK_PERIOD_MS); 
    
    vTaskDelay(200 / portTICK_PERIOD_MS); 

    current_state = STATE_GET_TIME;
}

// Estado responsável por obter a hora atual via NTP
void state_get_time(void) {
    if(!is_wifi_connected()) return; 
    
    initialize_time();

    // Aguardar a sincronização da hora
    int retry = 0;
    const int retry_count = 15;

    while (!is_time_synced() && ++retry < retry_count) {
        ESP_LOGI(TAG, "Aguardando sincronização de hora (%d/%d)...", retry, retry_count);
        vTaskDelay(1500 / portTICK_PERIOD_MS); 
    }

    if (!is_time_synced()) {
        ESP_LOGE(TAG, "Falha ao sincronizar a hora");
        
    } else {
        // Exibe a hora atual
        char time_str[64];
        get_current_time(time_str, sizeof(time_str));
        ESP_LOGI(TAG, "Hora atual obtida: %s", time_str);
        set_led_blink_frequency(250);
        calibrate_task_init(); // agenda uma calibração do RTC todo dia no mesmo horário
        init_custom_logger();
        current_state = STATE_WAIT_SCHEDULE;
    }
}

// Estado coringa utilizado nos testes
void state_await(void) {
    ESP_LOGI("AGUARDANDO", "Aguardando...");
    vTaskDelay(1500 / portTICK_PERIOD_MS); 
}

// Estado responsável monitorar os horários agendados
void state_await_schedule(void) {
    char time_str[64];

    scheduled_task_init();

    while(!flag_water_pump){
        if(flag_calibrate){
            current_state = STATE_GET_TIME;
            // vTaskSuspend(xHandleScheduledTask);
            // ESP_LOGI(TAG, "A task schedule foi suspensa.");
            vTaskSuspend(xHandleCalibrateTask);
            ESP_LOGI(TAG, "A task Calibrate foi suspensa.");
            break;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS); 
    }

    if(flag_water_pump){
        get_current_time(time_str, sizeof(time_str));
        ESP_LOGI(TAG, "Evento está ON, horário agendado atendido: %s", time_str);
        vTaskSuspend(xHandleScheduledTask);
        ESP_LOGI(TAG, "A task schedule foi suspensa.");
        current_state = STATE_PUMP_RUN;
    }
  
}

void state_pump_run(void){
    
    if(!flag_water_pump) return;
    
    activate_gpio_for_duration(pump_time);
    flag_water_pump = false;
    ESP_LOGI(TAG, "Water Pump Flag: %d", flag_water_pump);
    current_state = STATE_WAIT_SCHEDULE;

}


// Função que retorna uma string com o nome do estado
const char* state_to_string(state_t state) {
    switch (state) {
        case STATE_INIT:
            return "STATE_INIT";
        case STATE_GET_TIME:
            return "STATE_GET_TIME";
        case STATE_DONE:
            return "STATE_DONE";
        case STATE_AWAIT:
            return "STATE_AWAIT";
        case STATE_WAIT_SCHEDULE:
            return "STATE_WAIT_SCHEDULE";
        case STATE_PUMP_RUN:
            return "STATE_PUMP_RUN";    
        default:
            return "UNKNOWN_STATE";
    }
}


// Função para rodar a máquina de estados
void state_machine_run(void) {
    // Definir as funções de estado como ponteiros para função
    state_function_t state_functions[] = {
        state_init,
        state_get_time,
        state_await,
        state_await_schedule,
        state_pump_run,
        // state_done
    };

    while (current_state != STATE_DONE) {
        ESP_LOGI(TAG, "Estado atual: %s", state_to_string(current_state));
        state_functions[current_state](); // Executa o estado atual
        vTaskDelay(100 / portTICK_PERIOD_MS); // Pequeno delay para suavizar a transição
    }

    state_functions[STATE_DONE]();
}
