#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Motor.h"
#include "Key.h"
#include "Timer.h"
#include "PWM.h"
#include "Encoder.h"
#include "Serial.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int8_t KeyNum=1;
int16_t Speed1;//实际值
int16_t Speed2;//实际值
uint16_t Num_T = 0, Serial_T = 0, Key_T = 0, Motor_T = 0;
void System_Init(){
		Motor_Init();
		Key_Init();
		Timer_Init();
		Encoder_Init();
		Serial_Init();
}

int main(){
		
		System_Init();
	
		//OLED_ShowString(1, 1, "MODE:");
		/*
		while(1){
				OLED_ShowString(1, 1, "Count:");
				KeyNum = Key_GetNum();
				if(KeyNum == 1){
						sum+=1;
						OLED_ShowString(1, 7, "    ");
						OLED_ShowNum(1, 7, sum, 3);
				}
		}
		*/
		
}

//定时中断按键切换模式
void TIM2_IRQHandler(void){
		if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET){
				Num_T++;
				Motor_T++;
				Serial_T++;
				Speed1 = Encoder_Get1();
				Speed2 = Encoder_Get2();
				if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0){
						if(Key_T >= 1){
								KeyNum = -KeyNum;
								Key_T = 0;
						}
						Key_T++;
				}
				TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
		}
}
