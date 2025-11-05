#include "pid.h"

void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, int16_t output_max)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->Target = 0;
    pid->Error = 0;
    pid->LastError = 0;
    pid->PrevError = 0;
    pid->Integral = 0;
    pid->Output = 0;
    pid->OutputMax = output_max;
    pid->OutputMin = -output_max;
}

//速度控制
float PID_Incremental_Calculate(PID_TypeDef *pid, float target, float feedback)
{
    float increment;
    
    pid->Target = target;
    pid->Error = pid->Target - feedback;
    
    //增量式PID公式
    increment = pid->Kp * (pid->Error - pid->LastError) +
               pid->Ki * pid->Error + 
               pid->Kd * (pid->Error - 2 * pid->LastError + pid->PrevError);
    
    pid->Output += increment;
    
    //输出限幅
    if (pid->Output > pid->OutputMax)
        pid->Output = pid->OutputMax;
    else if (pid->Output < pid->OutputMin)
        pid->Output = pid->OutputMin;
    
    //更新误差历史
    pid->PrevError = pid->LastError;
    pid->LastError = pid->Error;
    
    return pid->Output;
}

//位置控制
float PID_Positional_Calculate(PID_TypeDef *pid, float target, float feedback)
{
    pid->Target = target;
    pid->Error = pid->Target - feedback;
    
    //积分项
    pid->Integral += pid->Error;
    
    //积分限幅
    if (pid->Integral > 1000) pid->Integral = 1000;
    if (pid->Integral < -1000) pid->Integral = -1000;
    
    //位置式PID公式
    pid->Output = pid->Kp * pid->Error +
                  pid->Ki * pid->Integral +
                  pid->Kd * (pid->Error - pid->LastError);
    
    //输出限幅
    if (pid->Output > pid->OutputMax)
        pid->Output = pid->OutputMax;
    else if (pid->Output < pid->OutputMin)
        pid->Output = pid->OutputMin;
    
    pid->LastError = pid->Error;
    
    return pid->Output;
}
