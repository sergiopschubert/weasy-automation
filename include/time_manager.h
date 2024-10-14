// time_manager.h
#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <stdio.h> 

#define MAX_SCHEDULED_TIMES 6

typedef struct {
    int hour;
    int minute;
    int second;
} scheduled_time_t;

extern scheduled_time_t scheduled_times[MAX_SCHEDULED_TIMES];
extern TaskHandle_t xHandleScheduledTask;
extern TaskHandle_t xHandleCalibrateTask;
extern bool flag_water_pump;
extern bool flag_calibrate;
// Função para inicializar o NTP e configurar o fuso horário
void initialize_time(void);

// Função para verificar se a hora foi sincronizada corretamente
bool is_time_synced(void);

// Função para obter a hora atual formatada
void get_current_time(char *time_str, size_t max_len);

void scheduled_task_init(void);

void calibrate_task_init(void);

#endif // TIME_MANAGER_H
