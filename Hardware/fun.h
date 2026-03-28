#ifndef __FUN_H
#define __FUN_H

#include "main.h"

void fun_led(void);
void fun_lcd(void);
void fun_mode(void);
float adc_read(ADC_HandleTypeDef *hadc);
void adc_proc(void);
void fun_data(void);


#endif
