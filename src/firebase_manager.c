#include "firebase_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "esp_http_client.h"
#include "wifi_manager.h"
#include "time_manager.h"
#include "credentials.h"


static bool in_logging_function = false;

void send_log_to_firebase(const char *log_message,const char *timestamp);

int my_log_vprintf(const char *fmt, va_list args);

void send_log_to_firebase(const char *log_message,const char *timestamp) {
    in_logging_function = true;

    // URL do Firebase Realtime Database

    // Criar objeto JSON
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        // ESP_LOGE(TAG, "Falha ao criar objeto JSON.");
        in_logging_function = false;
        return;
    }

    if (cJSON_AddStringToObject(root, "log", log_message) == NULL) {
        // ESP_LOGE(TAG, "Falha ao adicionar a mensagem de log ao objeto JSON.");
        cJSON_Delete(root);
        in_logging_function = false;
        return;
    }

    if (cJSON_AddStringToObject(root, "timestamp", timestamp) == NULL) {
        // ESP_LOGE(TAG, "Falha ao adicionar campo 'timestamp' ao JSON");
        in_logging_function = false;
        cJSON_Delete(root);
        return;
    }

    // Converter JSON para string
    char *post_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (post_data == NULL) {
        printf("Falha ao converter objeto JSON para string.");
        in_logging_function = false;
        return;
    }

    printf("Dados JSON: %s\n", post_data);

    // Configurar cliente HTTP
    esp_http_client_config_t config = {
        .url = FIREBASE_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        printf("Falha ao inicializar o cliente HTTP.\n");
        free(post_data);
        in_logging_function = false;
        return;
    }

    esp_err_t err = esp_http_client_set_header(client, "Content-Type", "application/json");

    if (err != ESP_OK) {
        printf("Erro ao definir cabeçalhos HTTP: %s\n", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        free(post_data);
        in_logging_function = false;
        return;
    }
  

    // Definir dados do POST
    err = esp_http_client_set_post_field(client, post_data, strlen(post_data));
    if (err != ESP_OK) {
        printf("Erro ao definir o campo POST: %s\n", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        free(post_data);
        in_logging_function = false;
        return;
    }

    // Executar requisição
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        printf("Resposta HTTP Status = %d\n", status_code);
        if (status_code == 200) {
            printf("Log enviado com sucesso ao Firebase\n");
        } else {
            printf("Falha ao enviar log. Status Code: %d\n", status_code);
        }
    } else {
        printf("Erro ao enviar log ao Firebase: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    free(post_data);
    in_logging_function = false;
}

int my_log_vprintf(const char *fmt, va_list args) {
    if(!in_logging_function){
        static char log_buffer[512];
    
        int len = vsnprintf(log_buffer, sizeof(log_buffer), fmt, args);

        if(len!=-1 && is_wifi_connected()){ 
            
            int pos_start = 9;
            int pos_end = (len - 6); 
            int len_substring = pos_end - pos_start + 1;
            char substring[len_substring+1];  
            char time_str[64];

            get_current_time(time_str, sizeof(time_str));
            strncpy(substring, log_buffer + pos_start, len_substring);
            substring[len_substring] = '\0'; 
            send_log_to_firebase(substring,time_str);
        }
    }
    return vprintf(fmt, args);    
}

void init_custom_logger(void) {
    esp_log_set_vprintf(&my_log_vprintf);
}
