/*
 * pid.c
 *
 *  Created on: 2026年3月8日
 *      Author: 杨金乐
 */
/*********************************************************************************************************************
*功能名称
*引脚定义   左后轮  PWM1: P21_2   PWM2: P21_3
*引脚定义   右后轮  PWM1: P21_4   PWM2: P21_5
*引脚定义   前转向  PWM1: P21_6   PWM2: P21_7
*硬件参数
*       同一 PWM 通道的多个引脚是互斥的，硬件上同一通道只有一套电路，没法同时驱动两个引脚输出
*       同一组内的所有通道，频率必须相同，占空比可各自独立设置
*       
*********************************************************************************************************************/
#include "zf_common_headfile.h"

#define PID_STEERING_ANGLE_DEADBAND_DEG    (0.30f)   // 角度死区，进入后认为到位
#define PID_STEERING_OUTPUT_DEADBAND        (8.0f)    // 输出死区，小于该值不驱动电机
#define PID_STEERING_OUTPUT_OFFSET             (30.0f)    // 速度输出偏置

PID_Struct_info left_wheel_pid_speed;
PID_Struct_info left_wheel_pid_acceleration;
PID_Struct_info right_wheel_pid_speed;
PID_Struct_info right_wheel_pid_acceleration;
PID_Struct_info steering_pid;

// #define STEERING_PWM_PIN_1      ATOM0_CH4_P02_4
// #define STEERING_PWM_PIN_2      ATOM0_CH5_P02_5
// #define LEFT_WHEEL_PWM_PIN_1    ATOM0_CH0_P21_2
// #define LEFT_WHEEL_PWM_PIN_2    ATOM0_CH1_P21_3
#define STEERING_PWM_PIN_1      ATOM1_CH4_P21_6
#define STEERING_PWM_PIN_2      ATOM1_CH5_P21_7
#define LEFT_WHEEL_PWM_PIN_1    ATOM0_CH0_P21_2
#define LEFT_WHEEL_PWM_PIN_2    ATOM0_CH1_P21_3
#define RIGHT_WHEEL_PWM_PIN_1   ATOM0_CH2_P21_4
#define RIGHT_WHEEL_PWM_PIN_2   ATOM0_CH3_P21_5

float target_steering_angle=0;//目标转角
float target_speed=0;//目标速度
uint8 control_mode=0;//控制模式 0仅转向控制 1自动控制

uint8 pid_init_flag=0xFF;//PID初始化标志位 0xFF未初始化 0已初始化
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PID控制初始化
// 参数说明
// 参数说明
// 使用说明
// 使用说明     开启6个通道的PWM
// 返回参数
// 使用示例     PWM频率为1000Hz 初始占空比为0 duty范围为0~PWM_DUTY_MAX(10000)
//-------------------------------------------------------------------------------------------------------------------
void pid_init(void)
{
    pwm_init(STEERING_PWM_PIN_1,1000,0); // 前转向PWM1
    pwm_init(STEERING_PWM_PIN_2,1000,0); // 前转向PWM2
    pwm_init(LEFT_WHEEL_PWM_PIN_1,1000,0); // 左后轮PWM1
    pwm_init(LEFT_WHEEL_PWM_PIN_2,1000,0); // 左后轮PWM2
    pwm_init(RIGHT_WHEEL_PWM_PIN_1,1000,0); // 右后轮PWM1
    pwm_init(RIGHT_WHEEL_PWM_PIN_2,1000,0); // 右后轮PWM2

    //PID参数初始化
    steering_pid.integral_limit=PID_INTEGRAL_LIMIT_STEERING;
    steering_pid.output_limit=PID_OUTPUT_LIMIT_STEERING;
    steering_pid.Kp=3.73960f; // 根据实际情况调整
    steering_pid.Ki=2.74780f;  // 根据实际情况调整
    steering_pid.Kd=1.28780f;  // 根据实际情况调整
    left_wheel_pid_speed.integral_limit=PID_INTEGRAL_LIMIT_SPEED;
    left_wheel_pid_speed.output_limit=PID_OUTPUT_LIMIT_SPEED;
    left_wheel_pid_acceleration.integral_limit=PID_INTEGRAL_LIMIT_ACCELERATION;
    left_wheel_pid_acceleration.output_limit=PID_OUTPUT_LIMIT_ACCELERATION;
    right_wheel_pid_speed.integral_limit=PID_INTEGRAL_LIMIT_SPEED;
    right_wheel_pid_speed.output_limit=PID_OUTPUT_LIMIT_SPEED;
    right_wheel_pid_acceleration.integral_limit=PID_INTEGRAL_LIMIT_ACCELERATION;
    right_wheel_pid_acceleration.output_limit=PID_OUTPUT_LIMIT_ACCELERATION;
    pid_control(0,0,0);
    if(ecoder_state_flag==1)
    {
        pit_ms_init(CCU60_CH0,5);
    }
    pid_init_flag=0xFF; // PID初始化完成
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     限幅函数 控制value在min和max之间
// 参数说明     value：需要限幅的值，指针类型，函数内部会修改这个值
// 参数说明     min：限幅的最小值
// 参数说明     max：限幅的最大值
// 使用说明     限幅对象为浮点数
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void pid_limit(float *value, float min, float max)
{
    if (*value > max)
        *value = max;
    else if (*value < min)
        *value = min;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     左轮控制函数 根据输入的占空比比例控制左轮转动方向和速度
// 参数说明     pwm_proportion：占空比比例，范围为-1.0到1.0，正值表示正转，负值表示反转，0表示停止
// 使用说明     占空比最大量程10000 浮点数0.0001对应占空比1
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void left_wheel_control(float pwm_proportion)
{
    // 根据速度设置PWM占空比，假设speed范围为-1.0到1.0
    pid_limit(&pwm_proportion, -1.0f, 1.0f); // 限制速度在-1.0到1.0之间
    uint32 duty=0;
    if(pwm_proportion>0)
    {
    duty = (uint32)(pwm_proportion*PWM_DUTY_MAX); // 左后轮正转
    pwm_set_duty(LEFT_WHEEL_PWM_PIN_1, duty); // 设置左后轮PWM1占空比
    pwm_set_duty(LEFT_WHEEL_PWM_PIN_2, 0); // 设置左后轮PWM2占空比
    }
    else if(pwm_proportion<0)
    {    
    duty = (uint32)(-pwm_proportion*PWM_DUTY_MAX); // 左后轮反转
    pwm_set_duty(LEFT_WHEEL_PWM_PIN_1, 0); // 设置左后轮PWM1占空比
    pwm_set_duty(LEFT_WHEEL_PWM_PIN_2, duty); // 设置左后轮PWM2占空比
    }
    else 
    {
    pwm_set_duty(LEFT_WHEEL_PWM_PIN_1, 0); // 停止左后轮
    pwm_set_duty(LEFT_WHEEL_PWM_PIN_2, 0); // 停止左后轮
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     右轮控制函数 根据输入的占空比比例控制右轮转动方向和速度
// 参数说明     pwm_proportion：占空比比例，范围为-1.0到1.0，正值表示正转，负值表示反转，0表示停止
// 使用说明     占空比最大量程10000 浮点数0.0001对应占空比1
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void right_wheel_control(float pwm_proportion)
{
    // 根据速度设置PWM占空比，假设speed范围为-1.0到1.0
    pid_limit(&pwm_proportion, -1.0f, 1.0f); // 限制速度在-1.0到1.0之间
    uint32 duty=0;
    if(pwm_proportion>0)
    {
    duty = (uint32)(pwm_proportion*PWM_DUTY_MAX); // 右后轮正转
    pwm_set_duty(RIGHT_WHEEL_PWM_PIN_1, duty); // 设置右后轮PWM1占空比
    pwm_set_duty(RIGHT_WHEEL_PWM_PIN_2, 0); // 设置右后轮PWM2占空比
    }
    else if(pwm_proportion<0)
    {    
    duty = (uint32)(-pwm_proportion*PWM_DUTY_MAX); // 右后轮反转
    pwm_set_duty(RIGHT_WHEEL_PWM_PIN_1, 0); // 设置右后轮PWM1占空比
    pwm_set_duty(RIGHT_WHEEL_PWM_PIN_2, duty); // 设置右后轮PWM2占空比
    }
    else 
    {
    pwm_set_duty(RIGHT_WHEEL_PWM_PIN_1, 0); // 停止右后轮
    pwm_set_duty(RIGHT_WHEEL_PWM_PIN_2, 0); // 停止右后轮
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      前轮转向控制函数 根据输入的占空比比例控制前轮转动力
// 参数说明      pwm_proportion：占空比比例，范围为-1.0到1.0，正值表示正转，负值表示反转，0表示停止
// 使用说明      占空比最大量程10000 浮点数0.0001对应占空比1
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void steering_control(float pwm_proportion)
{
    // 根据转向角度设置PWM占空比，假设angle范围为-1.0到1.0
    pid_limit(&pwm_proportion, -1.0f, 1.0f); // 限制转向角度在-1.0到1.0之间
    uint32 duty=0;
    if(pwm_proportion>0)
    {
    duty = (uint32)(pwm_proportion*PWM_DUTY_MAX); // 前转向右转
    pwm_set_duty(STEERING_PWM_PIN_1, duty); // 设置前转向PWM1占空比
    pwm_set_duty(STEERING_PWM_PIN_2, 0); // 设置前转向PWM2占空比
    }
    else if(pwm_proportion<0)
    {    
    duty = (uint32)(-pwm_proportion*PWM_DUTY_MAX); // 前转向左转
    pwm_set_duty(STEERING_PWM_PIN_1, 0); // 设置前转向PWM1占空比
    pwm_set_duty(STEERING_PWM_PIN_2, duty); // 设置前转向PWM2占空比
    }
    else 
    {
    pwm_set_duty(STEERING_PWM_PIN_1, 0); // 停止前转向
    pwm_set_duty(STEERING_PWM_PIN_2, 0); // 停止前转向
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PWM控制函数 封装前驱 后驱
// 参数说明     left_pwm_proportion：左轮占空比比例，范围为-1.0到1.0，正值表示正转，负值表示反转，0表示停止
// 参数说明     right_pwm_proportion：右轮占空比比例，范围为-1.0到1.0，正值表示正转，负值表示反转，0表示停止
// 参数说明     steering_pwm_proportion：前转向占空比比例，范围为-1.0到1.0，正值表示右转，负值表示左转，0表示停止
// 使用说明
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void pid_control(float left_pwm_proportion, float right_pwm_proportion, float steering_pwm_proportion)
{
    left_wheel_control(left_pwm_proportion);
    right_wheel_control(right_pwm_proportion);
    steering_control(steering_pwm_proportion);
}

// PID三环节 1返回当前值 2确定目标值 3PID公式处理
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PID1: 增量式pid更新函数 根据当前值和设定值计算PID输出
// 参数说明     pid：PID结构体指针
// 参数说明     current_value：当前值
// 使用说明     具有积分限幅和输出限幅功能
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void pid_update(PID_Struct_info *pid, float target_value)
{
    pid->set=target_value;
    pid->error =pid->set - pid->current_value;
    pid->integral += pid->error;
    pid_limit(&pid->integral, -pid->integral_limit, pid->integral_limit); // 限制积分项
    pid->differential= pid->error - pid->last_error;
    pid->output = pid->Kp * pid->error + pid->Ki * pid->integral + pid->Kd * pid->differential;
    pid_limit(&pid->output, -pid->output_limit, pid->output_limit); // 限制输出
    pid->last_error = pid->error;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PID2: 测量值返回函数 将编码器测量的当前值更新到PID结构体中
// 参数说明
// 使用说明    返回当前测量值到PID结构体的current_value字段中，供PID更新函数使用
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void pid_return_current(void)
{
    steering_pid.current_value=encoder_steering;//前轮转角
    left_wheel_pid_speed.current_value=encoder_left_wheel_speed;//左后轮线速度
    right_wheel_pid_speed.current_value=encoder_right_wheel_speed;//右后轮线速度
    left_wheel_pid_acceleration.current_value=encoder_left_wheel_acceleration;//左后轮加速度
    right_wheel_pid_acceleration.current_value=encoder_right_wheel_acceleration;//右后轮加速度
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     车辆角度控制函数    根据目标角度对前轮转向PID控制
// 参数说明     输入目标角度
// 使用说明     返回当前测量值到PID结构体的current_value字段中，供PID更新函数使用
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void pid_angle_control(float target_steering_angle)
{
    //前轮转向PID更新
    pid_limit(&target_steering_angle, PID_STEERING_MIN_ANGLE, PID_STEERING_MAX_ANGLE); // 限制目标转向角度在机械结构允许范围内

    // 输入死区 目标附近直接清零输出，避免积分残留导致电机持续爬行
    if(fabsf(target_steering_angle - steering_pid.current_value) <= PID_STEERING_ANGLE_DEADBAND_DEG)
    {
        steering_pid.error = 0.0f;
        steering_pid.last_error = 0.0f;
        steering_pid.differential = 0.0f;
        steering_pid.integral = 0.0f;
        steering_pid.output = 0.0f;
        steering_pid.set = target_steering_angle;
        return;
    }

    pid_update(&steering_pid, target_steering_angle);
    // 输出死区：小输出直接置零，防止电机在目标附近抖动/缓慢转动
    if(fabsf(steering_pid.output) < PID_STEERING_OUTPUT_DEADBAND)
    {
        steering_pid.output = 0.0f;
    }
    else
    {//输出偏移
        steering_pid.output +=PID_STEERING_OUTPUT_OFFSET;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     车辆速度控制函数    根据目标速度和当前转向角度计算左右轮的目标速度，并调用PID更新函数计算控制输出，
// 函数简介                        最后调用PWM控制函数输出控制信号
// 参数说明     输入目标速度
// 使用说明     返回当前测量值到PID结构体的current_value字段中，供PID更新函数使用
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void pid_speed_control(float target_speed)
{
    float left_speed = target_speed;
    float right_speed = target_speed;
    float steering_coff=0;
    // 调整后轮差速
    if(steering_pid.current_value > -1.0f && steering_pid.current_value < 1.0f)//避免tan函数在接近90度时出现极大值导致控制失效
    {
        left_speed= target_speed;
        right_speed= target_speed;
    }
    else
    {
        steering_coff=tanf(steering_pid.current_value*AtR)*PID_STEERING_COFF;
        left_speed=(1+steering_coff)*target_speed;
        right_speed=(1-steering_coff)*target_speed;
    }
    // 左右轮外环速度PID更新
    pid_update(&left_wheel_pid_speed, left_speed);
    pid_update(&right_wheel_pid_speed, right_speed);
    // 左右轮内环加速度PID更新
    pid_update(&left_wheel_pid_acceleration, left_wheel_pid_speed.output);
    pid_update(&right_wheel_pid_acceleration, right_wheel_pid_speed.output);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      车辆运动控制封装        手动模式和自动模式，手动下只调用速度控制函数，自动模式下调用角度控制函数和速度控制函数
// 函数简介                             自动模式 可用于遥控器控制 或者自动驾驶算法输出控制信号
// 参数说明      target_speed：目标速度
// 参数说明      target_steering_angle：目标转向角度
// 参数说明      control_mode：控制模式，0表示手动模式，1表示自动模式
// 使用说明      手动模式下target_steering_angle参数无效，自动模式下根据目标转向角度调整左右轮速度差，并控制前轮转向
// 返回参数
// 使用示例
//-------------------------------------------------------------------------------------------------------------------
void pid_motion_control(float target_speed, float target_steering_angle, uint8 control_mode)
{
    pid_return_current(); // 获取当前测量值更新到PID结构体中
    float steering_output=0;
    float left_acceleration_duty=0;
    float right_acceleration_duty=0;
    float steering_duty=0;
    if(control_mode==0) // 手动模式
    {
        pid_speed_control(target_speed);
        steering_output=0; // 手动模式下转向电机无力 PWM占空比为0
    }
    else if(control_mode==1) // 自动模式
    {
        pid_angle_control(target_steering_angle);
        pid_speed_control(target_speed);
        steering_output=steering_pid.output; // 自动模式下转向电机根据PID输出控制
    }
    else
    {
        // 无效控制模式，停止车辆
        pid_control(0, 0, 0);
    }
    //输出值处理 根据输出限幅计算 占空比比例
    //PWM占空比输出
    left_acceleration_duty=left_wheel_pid_acceleration.output/PID_ACC_MAX_OUTPUT; // 根据加速度PID输出计算左轮占空比比例
    right_acceleration_duty=right_wheel_pid_acceleration.output/PID_ACC_MAX_OUTPUT; // 根据加速度PID输出计算右轮占空比比例
    steering_duty=steering_output/PID_STEERING_MAX_OUTPUT; // 根据转向PID输出计算转向占空比比例
    pid_control(left_acceleration_duty, right_acceleration_duty, steering_duty);
    
}

void set_target_steering_angle(float angle)
{
    target_steering_angle=angle;
}
void set_target_speed(float speed)
{
    target_speed=speed;
}
void set_control_mode(uint8 mode)
{
    control_mode=mode;
}

void pid_loop(void)
{
    if(pid_init_flag==0)
    {
        pid_motion_control(target_speed, target_steering_angle, control_mode);
    }
}

    

