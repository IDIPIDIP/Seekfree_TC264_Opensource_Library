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
#define WHEELBASE_L 0.783 // 车辆前轴中心到后轴中心的垂直距离 (单位: 米)
#define MAX_MOTOR_SPEED 4.24f // 车辆最大速度 (单位: 米/秒)
#define MAX_MOTOR_PWM 10000 // 电机PWM占空比的最大值 (对应100%占空比)
#define TRACK_B 0.588f // 车辆左右轮之间的距离 (单位: 米)
#define MAIN_CONTROL_PERIOD (0.01f) // 主控制周期 (秒) - 10ms
#define REAR_SPEED_PID_KP (0.95f)
#define REAR_SPEED_PID_KI (0.22f)
#define REAR_SPEED_PID_KD (0.04f)

extern struct out_motor_speed motor_speed;// 输出的左右轮速度结构体
extern float n_speed ; // 车身期望中心速度 m/s
extern int dir; // 运动方向 1前进，0后退
extern struct pid_controller left_pid; // 左轮PID控制器
extern struct pid_controller right_pid; // 右轮PID控制器

//输入期望速度，dir1前进，2后退
void runpoint(float n_speed,int dir);
void Calculate_Differential_Speed(float turn_angle, float n_speed, struct out_motor_speed *out_motor_speed);
//x:点的x坐标数组 y:点的y坐标数组 point_dis:每点间距（实际点间距会小） start_index:拟合点的起始索引
void strline_fit(double *x, double *y, int point_dis,int start_index);

#endif /* CODE_TURN_H_ */
