// state_machine.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_task.h"
#include "state_machine.h"
#include "time_manager.h"
#include "esp_log.h"

static const char *TAG = "state_machine";

// Variável que armazena o estado atual
static state_t current_state = STATE_INIT;

// Definição das funções de estado
void state_init(void);
void state_get_time(void);


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
        default:
            return "UNKNOWN_STATE";
    }
}


// Implementação da função para o estado inicial
void state_init(void) {
    ESP_LOGI(TAG, "Estado: INIT");
    // Definir a frequência inicial do LED como 1 segundo
    set_led_blink_frequency(1000);
    // Próximo estado: piscar o LED rapidamente
    current_state = STATE_GET_TIME;
}

// Estado responsável por obter a hora atual via NTP
void state_get_time(void) {
    ESP_LOGI(TAG, "Estado: GET_TIME");

    // Inicializar SNTP e fuso horário
    initialize_time();

    // Aguardar a sincronização da hora
    int retry = 0;
    const int retry_count = 10;

    while (!is_time_synced() && ++retry < retry_count) {
        ESP_LOGI(TAG, "Aguardando sincronização de hora (%d/%d)...", retry, retry_count);
        vTaskDelay(1500 / portTICK_PERIOD_MS);  // Esperar 1.5 segundos entre tentativas
    }

    if (!is_time_synced()) {
        ESP_LOGE(TAG, "Falha ao sincronizar a hora");
        
    } else {
        // Exibe a hora atual
        char time_str[64];
        get_current_time(time_str, sizeof(time_str));
        ESP_LOGI(TAG, "Hora atual obtida: %s", time_str);
        set_led_blink_frequency(500);
        current_state = STATE_AWAIT;
    }
}
void state_await(void) {
    ESP_LOGI("AGUARDANDO", "Aguardando...");
    vTaskDelay(1500 / portTICK_PERIOD_MS); 
}



// Função para rodar a máquina de estados
void state_machine_run(void) {
    // Definir as funções de estado como ponteiros para função
    state_function_t state_functions[] = {
        state_init,
        state_get_time,
        state_await,
        // state_done
    };

    // Executar a função correspondente ao estado atual
    while (current_state != STATE_DONE) {
        state_functions[current_state](); // Executa o estado atual
        vTaskDelay(100 / portTICK_PERIOD_MS); // Pequeno delay para suavizar a transição
        ESP_LOGI("STATE_MACHINE", "Estado atual: %s", state_to_string(current_state));
    }

    // Executa o estado final (STATE_DONE)
    state_functions[STATE_DONE]();
}
