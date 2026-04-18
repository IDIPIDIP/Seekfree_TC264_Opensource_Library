#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

#ifndef CODE_ENCODER_H
#define CODE_ENCODER_H_

#define ENCODER_STEERING_PPR (22.0f) // 前轮转向编码器每转的脉冲数
#define ENCODER_STEERING_GEAR_RATIO (192.0f) // 前轮转向 编码器与转向轴的传动比(gear ratio)

#define ENCODER_WHEEL_PPR (512.0f) // 后轮编码器每转的脉冲数
#define ENCODER_WHEEL_GEAR_RATIO (7.2f)    // 后轮驱动 编码器与车轮的传动比(gear ratio)

/********************* 前轮转角快速计算宏 *********************/
// 前轮转角系数（预计算：360/(22*192)），编译期确定值，无运行时开销
#define ENCODER_STEERING_ANGLE_COEFF (360.0f / (ENCODER_STEERING_PPR * ENCODER_STEERING_GEAR_RATIO))
// 脉冲数n转前轮转角（度）
#define ENCODER_GET_STEER_ANGLE(n) ((float)(n) * ENCODER_STEERING_ANGLE_COEFF)

/********************* 后轮转速快速计算宏 *********************/
// 后轮转速系数（预计算：60/(512*7.2)），编译期确定值，无运行时开销
#define ENCODER_WHEEL_COEFF (6.2831853f / (ENCODER_WHEEL_PPR * ENCODER_WHEEL_GEAR_RATIO))
// 脉冲数n转后轮转角（弧度）
#define ENCODER_GET_WHEEL_COEFF(n) ((float)(n) * ENCODER_WHEEL_COEFF)

#define ENCODER_WHEEL_RADIUS (0.123f) // 车轮半径 单位：米

//后轮转速计算公式：后轮转速 = 脉冲数 * ENCODER_WHEEL_COEFF * encoder_pit_frequency
extern int16 encoder_pit_frequency;//5ms 200Hz刷新频率
extern int16 encoder_steering_pulse_count; // 前轮转向编码器脉冲计数
extern int16 encoder_left_wheel_5ms_speed; // 左后轮编码器脉冲计数
extern int16 encoder_right_wheel_5ms_speed; // 右后轮编码器脉冲计数

extern float encoder_steering;   //前轮转角 单位：度  
extern float encoder_left_wheel_speed; // 左后轮瞬时线速度
extern float encoder_right_wheel_speed; // 右后轮瞬时线速度
extern float encoder_left_wheel_acceleration; // 左后轮瞬时加速度
extern float encoder_right_wheel_acceleration; // 右后轮瞬时加速度

extern uint8 ecoder_state_flag; //编码器状态标志位 1未开启 0开启


//所需编码器函数 声明
void encoder_init(void);
void encoder_loop(void);
#endif /* CODE_ENCODER_H_ */
