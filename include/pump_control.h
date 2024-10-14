#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include "driver/gpio.h"

#define GPIO_OUTPUT_PIN  GPIO_NUM_4

void configure_gpio(void);
void activate_gpio_for_duration(uint32_t duration_ms);

#endif // PUMP_CONTROL_H