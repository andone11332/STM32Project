#include "stm32f10x.h"
#include "PWM.h"

void Motor_Init(void)
{
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    //电机1方向引脚 (PB12, PB13)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //电机2方向引脚 (PB14, PB15)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    PWM_Init();
}

void Motor1_SetPWM(int8_t Speed)
{
    if (Speed >= 0)
    {
        //正转
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
        GPIO_SetBits(GPIOB, GPIO_Pin_13);
        PWM_SetCompare1(Speed);
    }
    else
    {
        //反转
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
        GPIO_ResetBits(GPIOB, GPIO_Pin_13);
        PWM_SetCompare1(-Speed);
    }
}

void Motor2_SetPWM(int8_t Speed)
{
    if (Speed >= 0)
    {
        //正转
        GPIO_ResetBits(GPIOB, GPIO_Pin_14);
        GPIO_SetBits(GPIOB, GPIO_Pin_15);
        PWM_SetCompare2(Speed);
    }
    else
    {
        //反转
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
        GPIO_ResetBits(GPIOB, GPIO_Pin_15);
        PWM_SetCompare2(-Speed);
    }
}
