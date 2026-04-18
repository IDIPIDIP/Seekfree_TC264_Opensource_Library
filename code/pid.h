/*
 * pid.h
 *
 *  Created on: 2026年3月8日
 *      Author: 杨金乐
 */
#include "zf_common_typedef.h"

#ifndef CODE_PID_H_
#define CODE_PID_H_

#define WHEEL_BASE (0.783f) // 轴距，单位为米，根据实际机械结构调整
#define HALF_WHEEL_TRACK (0.588f/2.0f) // 轮距，单位为米，根据实际机械结构调整
#define PID_STEERING_COFF (HALF_WHEEL_TRACK/WHEEL_BASE) // 转向系数，根据车辆几何参数计算得到 轮距/轴距

#define PWM_DUTY_MAX     10000                 // PWM最大占空比  最大占空比越大占空比的步进值越小
#define PID_STEERING_MAX_ANGLE 28.0f   // 最大转向角度，单位为度，根据实际机械结构调整
#define PID_STEERING_MIN_ANGLE -28.0f  // 最小转向角度，单位为度，根据实际机械结构调整
 
#define PID_DUTY_MAX     GTM_ATOM0_PWM_DUTY_MAX                 // PWM最大占空比  最大占空比越大占空比的步进值越小

#define PID_INTEGRAL_LIMIT_STEERING 200.0f //转向积分限幅
#define PID_OUTPUT_LIMIT_STEERING 500.0f //转向输出限幅
#define PID_INTEGRAL_LIMIT_SPEED 100.0f //速度积分限幅
#define PID_OUTPUT_LIMIT_SPEED 7.0f  //速度输出限幅
#define PID_INTEGRAL_LIMIT_ACCELERATION 200.0f //加速度积分限幅
#define PID_OUTPUT_LIMIT_ACCELERATION 500.0f  //加速度输出限幅

#define PID_STEERING_MAX_OUTPUT 300.0f //转向PID最大输出，根据实际情况调整
#define PID_ACC_MAX_OUTPUT 1500.0f //速度PID最大输出，根据实际情况调整


typedef struct 
{
    float current_value; // 当前值
    float error;         // 当前误差
    float last_error; // 上一次的误差
    float output;       // 输出值
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数
    float set;     // 目标值
    float integral;     // 积分项
    float differential;   // 微分项
    float integral_limit; // 积分限幅
    float output_limit; // 输出限幅
}PID_Struct_info;

extern PID_Struct_info left_wheel_pid_speed;
extern PID_Struct_info left_wheel_pid_acceleration;
extern PID_Struct_info right_wheel_pid_speed;
extern PID_Struct_info right_wheel_pid_acceleration;
extern PID_Struct_info steering_pid;
extern float target_speed;
extern float target_steering_angle;
extern uint8 control_mode;

//开启PWM端口
void pid_init(void);
void pid_control(float left_pwm_proportion, float right_pwm_proportion, float steering_pwm_proportion);
void pid_limit(float *value, float min, float max);
// 放中断里调用的控制函数 频率200Hz，5ms调用一次
void pid_motion_control(float target_speed, float target_steering_angle, uint8 control_mode);
void set_target_steering_angle(float angle);
void set_target_speed(float speed);
void set_control_mode(uint8 mode);
#endif /* CODE_PID_H_ */
