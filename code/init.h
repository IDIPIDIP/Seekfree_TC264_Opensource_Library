/*
 * init.h
 *
 *  Created on: 2026?2?27?
 *      Author: IDIPIDIP
 */

#ifndef CODE_INIT_H_
#define CODE_INIT_H_

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

#define ENCODER_PULSE_PER_REV 2048 //?????????
#define KEY0_PIN 
#define KEY1_PIN 
#define KEY2_PIN 
#define KEY3_PIN 
#define speaker_pwm_channel ATOM1_CH2_P33_11//???PWM??

extern int L_PWM_P;//??PWM??
extern int L_PWM_K;//??PWM???? =PWM_K/10000
extern int R_PWM_P;//???PWM??
extern int R_PWM_K;//???PWM???? =PWM_K/10000
extern int l_encoder;//???????
extern int r_encoder;//????????
extern int f_encoder;//?????????

void init();

#endif /* CODE_INIT_H_ */
