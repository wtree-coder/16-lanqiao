#include "main.h"
#include "key.h"

uint8_t get_state(uint8_t id){
    if(id == 0) return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
    if(id == 1) return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
    if(id == 2) return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
    if(id == 3) return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    return 1;
}

uint8_t key_long[4];
uint8_t key_short[4];

void key_tick(void){
    static uint16_t key_time[4];
    static uint8_t key_state[4];

    for(int i = 0;i < 4;i++){
        switch(key_state[i]){
            case 0:
                if(get_state(i) == 0) key_state[i] = 1;
                break;
            case 1:
                if(get_state(i) == 0){
                    key_state[i] = 2;
                    key_time[i] = 0;
                }
                else key_state[i] = 0;
                break;
            case 2:
                if(get_state(i) == 0) key_time[i]++;
                else key_state[i] = 3;
                break;
            case 3:
                if(get_state(i) == 0) key_state[i] = 2;
                else{
                    if(key_time[i] > 200) key_long[i] = 1;
                    else key_short[i] = 1;
                    key_state[i] = 0;
                }
                break;
        }
    }
}