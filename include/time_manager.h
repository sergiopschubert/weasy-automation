// time_manager.h
#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <stdio.h> 
// Função para inicializar o NTP e configurar o fuso horário
void initialize_time(void);

// Função para verificar se a hora foi sincronizada corretamente
bool is_time_synced(void);

// Função para obter a hora atual formatada
void get_current_time(char *time_str, size_t max_len);

#endif // TIME_MANAGER_H
