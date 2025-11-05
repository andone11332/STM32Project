#ifndef __PID_H
#define __PID_H

#include <stdint.h>

typedef struct
{
    float Target;       
    float Kp;           
    float Ki;           
    float Kd;          
    float Error;        
    float LastError;    
    float PrevError;    
    float Integral;     
    float Output;       
    int16_t OutputMax;  
    int16_t OutputMin;
} PID_TypeDef;

void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, int16_t output_max);
float PID_Incremental_Calculate(PID_TypeDef *pid, float target, float feedback);
float PID_Positional_Calculate(PID_TypeDef *pid, float target, float feedback);

#endif
