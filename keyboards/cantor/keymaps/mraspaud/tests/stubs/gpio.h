#pragma once
#include <stdbool.h>
typedef int pin_t;
void gpio_set_pin_input_high(pin_t pin);
bool gpio_read_pin(pin_t pin);
