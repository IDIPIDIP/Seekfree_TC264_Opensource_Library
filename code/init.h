/*
 * init.h
 *
 *  Created on: 2026年2月27日
 *      Author: IDIPIDIP
 */

#ifndef CODE_INIT_H_
#define CODE_INIT_H_

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

#define ENCODER_PULSE_PER_REV 2048 //编码器每转的脉冲数
#define KEY0_PIN 
#define KEY1_PIN 
#define KEY2_PIN 
#define KEY3_PIN 

extern int L_PWM_P;//左电机PWM频率
extern int L_PWM_K;//左电机PWM占空比 =PWM_K/10000
extern int R_PWM_P;//右电机PWM频率
extern int R_PWM_K;//右电机PWM占空比 =PWM_K/10000
extern int l_encoder;//左后编码器计数值
extern int r_encoder;//右后编码器计数值
extern int f_encoder;//前轮编码器计数值

void init();

#endif /* CODE_INIT_H_ */
