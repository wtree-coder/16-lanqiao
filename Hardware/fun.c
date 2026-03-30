#include "main.h"
#include "fun.h"
#include <stdio.h>
#include "lcd.h"
#include "key.h"
#include "led.h"
#include "adc.h"
#include <stdlib.h>

uint8_t lcd_mode = 0;
char buf[30];
int count_time = 0;
uint8_t lock_flag = 0;
float adc_R38,    //频率
adc_R37;          //占空比

int DS = 1,DR = 80,FS = 100,FR = 2000;
int active_DS = 1, active_DR = 80, active_FS = 100, active_FR = 2000;

uint16_t free_A;
int n_DS, n_FS;
float end1,end2;
uint8_t work_flag = 1, update_flag = 1;
uint16_t CF_c,DF_c;
int CD_t;


void fun_led(void){
    if(lcd_mode == 0 || lcd_mode == 3) led_show(1, 1);
    else led_show(1, 0);

    if(lock_flag) led_show(2, 1);
    else led_show(2, 0);

    if(work_flag == 0) led_show(3, 1);
    else led_show(3, 0);
}

void fun_mode(void){
    static int index = 0;
    switch(lcd_mode){
        case 0://监控UNLOCK
            if(key_short[0]){
                key_short[0] = 0;
                lcd_mode = 1;
            }
            if(key_short[1]){
                key_short[1] = 0;
                lock_flag = 1;
                lcd_mode = 3;
            }
            if(key_long[1]){
                key_long[1] = 0;
                count_time = 0;
            }
            break;
        case 1://统计
            if(key_short[0]){
                key_short[0] = 0;
                lcd_mode = 2;
                index = 0;
            }
            break;
        case 2://参数,(调参数)
            if(key_short[0]){
                key_short[0] = 0;
                lcd_mode = 0;

            if((DR >= DS + 10) && (FR >= FS + 1000)){
                active_DS = DS;
                active_DR = DR;
                active_FS = FS;
                active_FR = FR;
            }else{
                DS = active_DS;
                DR = active_DR;
                FS = active_FS;
                FR = active_FR;
            }
        }
            if(key_short[1]){
                key_short[1] = 0;
                index++;
                if(index == 4) index = 0;
            }
            if(index == 0){
                if(key_short[2]){ key_short[2]=0; DS++; if(DS>10) DS=10; }
                if(key_short[3]){ key_short[3]=0; DS--; if(DS<1) DS=1; }
            }
            if(index == 1){
                if(key_short[2]){ key_short[2]=0; DR+=10; if(DR>90) DR=90; }
                if(key_short[3]){ key_short[3]=0; DR-=10; if(DR<10) DR=10; }
            }
            if(index == 2){
                if(key_short[2]){ key_short[2]=0; FS+=100; if(FS>10000) FS=10000; }
                if(key_short[3]){ key_short[3]=0; FS-=100; if(FS<100) FS=100; }
            }
            if(index == 3){
                if(key_short[2]){ key_short[2]=0; FR+=1000; if(FR>10000) FR=10000; }
                if(key_short[3]){ key_short[3]=0; FR-=1000; if(FR<1000) FR=1000; }
            }
            break;
        case 3:
            if(key_short[0]) key_short[0] = 0;
            if(key_short[1]){
                key_short[1] = 0;
                lcd_mode = 0;
                lock_flag = 0;
            }
            if(key_short[2]) key_short[2] = 0;
            if(key_short[3]) key_short[3] = 0;
            if(key_long[1]){
                key_long[1] = 0;
                count_time = 0;
            }
            break;
        }
}

float adc_read(ADC_HandleTypeDef *hadc){
    HAL_ADC_Start(hadc);
    float value = HAL_ADC_GetValue(hadc);
    return 3.3f*value/4095;
}

void adc_proc(void){
    adc_R38 = adc_read(&hadc1);
    adc_R37 = adc_read(&hadc2); //占空比
}

void fun_data(void){
    n_DS = (active_DR - 10) / active_DS;
    n_FS = (active_FR - 1000) / active_FS;
    
    if(lock_flag == 0){
        end1 = 10 + (int)((float)adc_R37 / 3.3f * n_DS) * active_DS;
        if(end1 > active_DR) end1 = active_DR;
        TIM3->CCR2 = (int)end1;

        end2 = 1000 + (int)((float)adc_R38 / 3.3f * n_FS) * active_FS;
        if(end2 > active_FR) end2 = active_FR;
        TIM3->PSC = 1000000/(int)end2-1;
    }

    if(abs((int)free_A - (int)end2) >= 1000){
        work_flag = 0;
        if(update_flag){
            CF_c = (int)end2;
            CD_t = (int)end1;
            DF_c = free_A;
            update_flag = 0;
        }
    }
    else{
        work_flag = 1;
        update_flag = 1;
    }
}

void fun_lcd(void){  
    static uint8_t last_lcd_mode = 0;
    static uint8_t last_lock_flag = 0;  
    if(last_lcd_mode != lcd_mode){
        LCD_Clear(Black);
        last_lcd_mode = lcd_mode;
    }   
    uint8_t temp = GPIOC->ODR;
    switch(lcd_mode){
        case 0://监控UNLOCK
            sprintf(buf, "       PWM          ");
            LCD_DisplayStringLine(Line1, (uint8_t *)buf);
            sprintf(buf, "   CF=%dHz          ", (int)end2);
            LCD_DisplayStringLine(Line3, (uint8_t *)buf);
            sprintf(buf, "   CD=%d%%          ",(int)end1);//占空比
            LCD_DisplayStringLine(Line4, (uint8_t *)buf);
            sprintf(buf, "   DF=%dHz          ", free_A);
            LCD_DisplayStringLine(Line5, (uint8_t *)buf);
            if(last_lock_flag != lock_flag){
                last_lock_flag = lock_flag;
                LCD_Clear(Black);
            }                
            sprintf(buf, "   ST=UNLOCK          ");
            LCD_DisplayStringLine(Line6, (uint8_t *)buf);
            sprintf(buf, "   %02dH%02dM%02dS          ",
                count_time/3600, count_time%3600/60, count_time%60);
            LCD_DisplayStringLine(Line7, (uint8_t *)buf);
            GPIOC->ODR = temp;
            break;
        case 1://统计
            sprintf(buf, "       RECD          ");
            LCD_DisplayStringLine(Line1, (uint8_t *)buf);
            sprintf(buf, "   CF=%dHz          ",CF_c);
            LCD_DisplayStringLine(Line3, (uint8_t *)buf);
            sprintf(buf, "   CD=%d%%          ",CD_t);
            LCD_DisplayStringLine(Line4, (uint8_t *)buf);
            sprintf(buf, "   DF=%dHz          ",DF_c);
            LCD_DisplayStringLine(Line5, (uint8_t *)buf);
            sprintf(buf, "   XF=%dHz          ",abs(CF_c - DF_c));
            LCD_DisplayStringLine(Line6, (uint8_t *)buf);
            sprintf(buf, "   %02dH%02dM%02dS          ",
                count_time/3600, count_time%3600/60, count_time%60);
            LCD_DisplayStringLine(Line7, (uint8_t *)buf);
            GPIOC->ODR = temp;
            break;
        case 2://参数
            sprintf(buf, "       PARA          ");
            LCD_DisplayStringLine(Line1, (uint8_t *)buf);
            sprintf(buf, "   DS=%d%%          ",DS);
            LCD_DisplayStringLine(Line3, (uint8_t *)buf);
            sprintf(buf, "   DR=%d%%          ",DR);
            LCD_DisplayStringLine(Line4, (uint8_t *)buf);
            sprintf(buf, "   FS=%dHz          ",FS);
            LCD_DisplayStringLine(Line5, (uint8_t *)buf);
            sprintf(buf, "   FR=%dHz          ",FR);
            LCD_DisplayStringLine(Line6, (uint8_t *)buf);
            break;
        case 3://LOCK
            sprintf(buf, "       PWM          ");
            LCD_DisplayStringLine(Line1, (uint8_t *)buf);
            sprintf(buf, "   CF=%dHz          ", (int)end2);
            LCD_DisplayStringLine(Line3, (uint8_t *)buf);
            sprintf(buf, "   CD=%d%%          ",(int)end1);//占空比
            LCD_DisplayStringLine(Line4, (uint8_t *)buf);
            sprintf(buf, "   DF=%dHz          ", free_A);
            LCD_DisplayStringLine(Line5, (uint8_t *)buf);
            if(last_lock_flag != lock_flag){
                last_lock_flag = lock_flag;
                LCD_Clear(Black);
            }                
            sprintf(buf, "   ST=LOCK          ");
            LCD_DisplayStringLine(Line6, (uint8_t *)buf);
            sprintf(buf, "   %02dH%02dM%02dS          ",
                count_time/3600, count_time%3600/60, count_time%60);
            LCD_DisplayStringLine(Line7, (uint8_t *)buf);
            GPIOC->ODR = temp;
            break;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if(htim->Instance == TIM17){
        if(lock_flag == 0){
            count_time++;
        }
    }
    if(htim->Instance == TIM16){
        fun_mode();
        key_tick();
        fun_data();
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
    if(htim->Instance == TIM2){
        free_A = 1000000/(TIM2->CCR1+1);
        TIM2->CNT = 0;
    }
}

