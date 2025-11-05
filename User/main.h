#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"

//系统模式定义
#define MODE_SPEED_CONTROL   0  //速度控制
#define MODE_POSITION_SYNC  1  //位置同步

//速度控制参数
#define SPEED_SAMPLE_TIME   10  //速度采样时间间隔
#define SPEED_PID_KP        2.0f
#define SPEED_PID_KI        0.5f
#define SPEED_PID_KD        0.1f
#define SPEED_MAX           80  //最大速度限制

//位置同步参数
#define POSITION_PID_KP     3.0f
#define POSITION_PID_KI     0.1f
#define POSITION_PID_KD     0.5f

//全局变量声明
extern uint8_t SystemMode;
extern int16_t TargetSpeed;
extern int32_t Motor1_Position, Motor2_Position;

//函数声明
void TIM2_IRQHandler(void);
void System_Init(void);
void SpeedControl_Update(void);
void PositionSync_Update(void);
void ProcessUartCommand(char *cmd);

#endif
