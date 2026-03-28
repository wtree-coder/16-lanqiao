#include "main.h"
#include "led.h"

void led_show(uint8_t led, uint8_t state){
    static uint8_t ledbuf = 0xff;
    if(state){
        ledbuf &=~ (1 << (led-1));
    }else{
        ledbuf |= (1 << (led-1));
    }

    GPIOC->ODR = ledbuf << 8;
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}
