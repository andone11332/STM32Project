#ifndef __ENCODER_H
#define __ENCODER_H

void Encoder_Init(void);
int16_t Encoder1_Get(void);  //电机1编码器 (PA6, PA7)
int16_t Encoder2_Get(void);  //电机2编码器 (PB6, PB7)

#endif
