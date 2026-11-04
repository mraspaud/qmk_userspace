#pragma once
#include <stdint.h>
typedef struct {void(*set_color)(int,uint8_t,uint8_t,uint8_t);void(*set_color_all)(uint8_t,uint8_t,uint8_t);void(*flush)(void);} rgblight_driver_t;
extern const rgblight_driver_t rgblight_driver;
