// state_machine.h
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

// Definir os estados da máquina
typedef enum {
    STATE_INIT,      // Estado inicial
    STATE_GET_TIME,  // Estado que pisca o LED rapidamente
    STATE_AWAIT,  // Estado que pisca o LED lentamente
    STATE_DONE       // Estado final
} state_t;

// Definir o tipo de função que cada estado irá executar
typedef void (*state_function_t)(void);

// Função para iniciar a máquina de estados
void state_machine_run(void);

#endif // STATE_MACHINE_H
