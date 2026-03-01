/*
 * turn.h
 *
 *  Created on: 2026年2月27日
 *      Author: IDIPIDIP
 */

#ifndef CODE_TURN_H_
#define CODE_TURN_H_

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

extern struct out_motor_speed motor_speed;// 输出的左右轮速度结构体
extern float n_speed ; // 车身期望中心速度 m/s
extern int dir; // 运动方向 1前进，0后退

//输入期望速度，dir1前进，2后退
void runpoint(float n_speed,int dir);
void Calculate_Differential_Speed(float turn_angle, float n_speed, struct out_motor_speed *out_motor_speed);


#endif /* CODE_TURN_H_ */
