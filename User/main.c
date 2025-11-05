#include "stm32f10x.h"
#include "OLED.h"
#include "Serial.h"
#include "Motor.h"
#include "Encoder.h"
#include "Key.h"
#include "Delay.h"
#include "pid.h"
#include "main.h"
#include<string.h>
#include<stdlib.h>

// 全局变量定义
uint8_t SystemMode = MODE_SPEED_CONTROL;
int16_t TargetSpeed = 0;
int32_t Motor1_Position = 0, Motor2_Position = 0;

// PID控制器
PID_TypeDef SpeedPID1, SpeedPID2;
PID_TypeDef PositionPID;

// 速度计算变量
int16_t Speed1 = 0, Speed2 = 0;
uint32_t SpeedUpdateTime = 0;

// 串口命令缓冲区
char UartRxBuffer[64];
uint8_t UartRxIndex = 0;

void System_Init(void)
{
    // 初始化所有外设
    OLED_Init();
    Serial_Init();
    Motor_Init();
    Encoder_Init();
    Key_Init();
    
    // 初始化PID控制器
    PID_Init(&SpeedPID1, SPEED_PID_KP, SPEED_PID_KI, SPEED_PID_KD, SPEED_MAX);
    PID_Init(&SpeedPID2, SPEED_PID_KP, SPEED_PID_KI, SPEED_PID_KD, SPEED_MAX);
    PID_Init(&PositionPID, POSITION_PID_KP, POSITION_PID_KI, POSITION_PID_KD, 80);
    
    // 初始化定时器2用于速度采样(10ms中断)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 10000 - 1;  // 10ms
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;  // 72MHz/72 = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM_Cmd(TIM2, ENABLE);
    
    // 显示初始信息
    OLED_ShowString(0, 0, "双电机控制系统", OLED_8X16);
    OLED_ShowString(0, 2, "模式:速度控制", OLED_8X16);
    OLED_ShowString(0, 4, "目标速度:   0", OLED_8X16);
    OLED_ShowString(0, 6, "实际速度: 0  0", OLED_8X16);
    
    Serial_Printf("System Initialized\r\n");
    Serial_Printf("Commands: @speed xxx, @mode 0/1\r\n");
}

// 定时器2中断服务函数 - 10ms速度采样
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
        // 读取编码器值并计算速度
        Speed1 = Encoder1_Get();  // 10ms内的计数 = 速度
        Speed2 = Encoder2_Get();
        
        // 更新位置(用于位置同步模式)
        Motor1_Position += Speed1;
        Motor2_Position += Speed2;
        
        // 串口发送速度数据到上位机
        Serial_Printf("!SPEED:%d,%d,%d\r\n", TargetSpeed, Speed1, Speed2);
        
        // 根据模式更新控制
        if (SystemMode == MODE_SPEED_CONTROL)
        {
            SpeedControl_Update();
        }
        else
        {
            PositionSync_Update();
        }
    }
}

// 速度控制更新
void SpeedControl_Update(void)
{
    if (TargetSpeed == 0)
    {
        // 目标速度为0时，停止电机
        Motor1_SetPWM(0);
        Motor2_SetPWM(0);
    }
    else
    {
        // 使用PID计算PWM输出
        int16_t pwm1 = PID_Incremental_Calculate(&SpeedPID1, TargetSpeed, Speed1);
        int16_t pwm2 = PID_Incremental_Calculate(&SpeedPID2, TargetSpeed, Speed2);
        
        Motor1_SetPWM(pwm1);
        Motor2_SetPWM(pwm2);
    }
}

// 位置同步更新
void PositionSync_Update(void)
{
    // 电机2跟随电机1的位置
    int16_t pwm2 = PID_Positional_Calculate(&PositionPID, Motor1_Position, Motor2_Position);
    Motor2_SetPWM(pwm2);
    
    // 电机1自由转动（手动控制）
    Motor1_SetPWM(0);  // 不主动驱动电机1
}

// 处理串口命令
void ProcessUartCommand(char *cmd)
{
    if (strncmp(cmd, "@speed", 6) == 0)
    {
        int speed = atoi(cmd + 7);
        if (speed >= -100 && speed <= 100)
        {
            TargetSpeed = speed;
            OLED_ShowSignedNum(80, 4, TargetSpeed, 4, OLED_8X16);
            Serial_Printf("Target speed set to: %d\r\n", TargetSpeed);
        }
    }
    else if (strncmp(cmd, "@mode", 5) == 0)
    {
        uint8_t mode = atoi(cmd + 6);
        if (mode == 0 || mode == 1)
        {
            SystemMode = mode;
            OLED_ShowString(40, 2, mode == 0 ? "速度控制" : "位置同步", OLED_8X16);
            
            // 重置PID和位置
            if (mode == MODE_POSITION_SYNC)
            {
                Motor1_Position = 0;
                Motor2_Position = 0;
                PositionPID.Integral = 0;
                PositionPID.LastError = 0;
            }
            
            Serial_Printf("Mode changed to: %s\r\n", mode == 0 ? "Speed Control" : "Position Sync");
        }
    }
}

int main(void)
{
    System_Init();
    
    while (1)
    {
        // 按键处理 - 切换模式
        Key_Tick();
        if (Key_GetNum() == 1)
        {
            SystemMode = !SystemMode;
            OLED_ShowString(40, 2, SystemMode == 0 ? "速度控制" : "位置同步", OLED_8X16);
            
            if (SystemMode == MODE_POSITION_SYNC)
            {
                // 位置同步模式：重置位置
                Motor1_Position = 0;
                Motor2_Position = 0;
                PositionPID.Integral = 0;
                PositionPID.LastError = 0;
            }
            
            Delay_ms(300); // 防抖
        }
        
        // 串口命令处理
        if (Serial_GetRxFlag())
        {
            uint8_t data = Serial_GetRxData();
            
            if (data == '\r' || data == '\n')
            {
                if (UartRxIndex > 0)
                {
                    UartRxBuffer[UartRxIndex] = '\0';
                    ProcessUartCommand(UartRxBuffer);
                    UartRxIndex = 0;
                }
            }
            else if (UartRxIndex < sizeof(UartRxBuffer) - 1)
            {
                UartRxBuffer[UartRxIndex++] = data;
            }
        }
        
        // OLED显示更新
        OLED_ShowSignedNum(80, 6, Speed1, 4, OLED_8X16);
        OLED_ShowSignedNum(104, 6, Speed2, 4, OLED_8X16);
        
        if (SystemMode == MODE_POSITION_SYNC)
        {
            OLED_ShowSignedNum(0, 4, Motor1_Position/100, 6, OLED_8X16);
            OLED_ShowSignedNum(80, 4, Motor2_Position/100, 6, OLED_8X16);
        }
        
        Delay_ms(10);
    }
}
