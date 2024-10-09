// wifi_manager.h
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"

// Função para inicializar o Wi-Fi
void wifi_init_sta(void);

// Função para criar a task de Wi-Fi
void wifi_start_task(void);

bool is_wifi_connected(void);

// extern bool wifi_connected;

#endif // WIFI_MANAGER_H
