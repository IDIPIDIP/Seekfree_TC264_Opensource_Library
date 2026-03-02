/*
 * turn.c
 *
 *  Created on: 2026锟斤拷2锟斤拷27锟斤拷
 *      Author: IDIPIDIP
 */

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

int fit_point_num = 0; // 拟合点数量

float turn_angle = 0.0f; // 前轮转向角 (度, +左拐, -右拐)
float n_speed = 3.0f; // 车身期望中心速度 m/s
int dir = 1; // 运动方向 1前进，0后退
int lmotor_speed = 83; // 左轮电机满转速
int rmotor_speed = 83; // 右轮电机满转速 车速=rmotor_speed/20*0.77
struct out_motor_speed motor_speed;// 输出的左右轮速度结构体

// ===================== PID 控制结构体 =====================
struct pid_controller
{
    float kp;           // 比例系数
    float ki;           // 积分系数
    float kd;           // 微分系数
    float integral;     // 积分项累积值
    float last_error;   // 上一次误差值
    float output_limit; // 输出限值
};

// 左右轮 PID 控制器
struct pid_controller left_pid = {REAR_SPEED_PID_KP, REAR_SPEED_PID_KI, REAR_SPEED_PID_KD, 0.0f, 0.0f, MAX_MOTOR_SPEED};
struct pid_controller right_pid = {REAR_SPEED_PID_KP, REAR_SPEED_PID_KI, REAR_SPEED_PID_KD, 0.0f, 0.0f, MAX_MOTOR_SPEED};




struct out_motor_speed
{
    float l_speed;//左轮速度 (m/s)
    float r_speed;//右轮速度 (m/s)
};

// ===================== 获取实际速度接口 =====================
// 注意: 需要根据你的编码器或速度估计器接口修改这两个函数
float get_left_motor_speed(void)
{
    // TODO: 从编码器或速度观测器读取实际左轮速度 (m/s)
    float left_speed = (float)l_encoder / ENCODER_PULSE_PER_REV * 0.77f*100; //读取编码器计数值并转换为速度 (m/s) 0.77是轮子周长/编码器每转脉冲数 10ms故*100
    l_encoder = 0; // 读取后清零计数值
    return left_speed;
}

float get_right_motor_speed(void)
{
    // TODO: 从编码器或速度观测器读取实际右轮速度 (m/s)
    float right_speed = (float)r_encoder / ENCODER_PULSE_PER_REV * 0.77f*100;
    r_encoder = 0; // 读取后清零计数值
    return right_speed;
}

//前轮转向,返回°
float get_front_wheel_angle(void)
{
    float front_wheel_angle = (float)f_encoder / ENCODER_PULSE_PER_REV * 360.0f; // 锟斤拷锟斤拷前锟斤拷转锟斤拷嵌确锟轿拷锟?360锟饺ｏ拷锟斤拷锟斤拷锟斤拷每转2048锟斤拷锟斤拷
    return front_wheel_angle;
}

// ===================== PID 控制函数 =====================
void pid_init(struct pid_controller *pid, float kp, float ki, float kd, float output_limit)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->output_limit = output_limit;
}

float pid_calculate(struct pid_controller *pid, float target, float actual, float dt)
{
    float error = target - actual;
    
    // P项
    float p_output = pid->kp * error;
    
    // I项 - 带积分限制防止积分饱和
    pid->integral += error * dt;
    if(pid->integral > pid->output_limit)
        pid->integral = pid->output_limit;
    if(pid->integral < -pid->output_limit)
        pid->integral = -pid->output_limit;
    float i_output = pid->ki * pid->integral;
    
    // D项
    float d_output = pid->kd * (error - pid->last_error) / dt;
    pid->last_error = error;
    
    // 总输出
    float output = p_output + i_output + d_output;
    
    // 输出限值处理
    if(output > pid->output_limit)
        output = pid->output_limit;
    if(output < -pid->output_limit)
        output = -pid->output_limit;
    
    return output;
}
// PID 重置函数 (停止时调用)
void pid_reset(struct pid_controller *pid)
{
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
}

/**
 * @brief 计算转向时的左右轮差速
 * @param turn_angle 前轮转向角 (度, +左拐, -右拐)
 * @param n_speed  车身期望中心速度 (建议范围: 0 ~ MAX_MOTOR_SPEED)
 * @param out_motor_speed 输出的左右轮速度结构体
 * @param WHEELBASE_L 车辆前轴中心到后轴中心的垂直距离 (单位: 米)
 * @param TRACK_B 车辆左右轮之间的距离 (单位: 米)
 */
void Calculate_Differential_Speed(float turn_angle, float n_speed, struct out_motor_speed *out_motor_speed)
{
    const float kDegToRad = 0.01745329252f;
    float turn_angle_rad = turn_angle * kDegToRad;// 转换为弧度

    // 1. 处理直行情况 (避免tan(0)和除以零)
    if (turn_angle > -3.0f && turn_angle < 3.0f)
    {
        out_motor_speed->l_speed  = n_speed;
        out_motor_speed->r_speed = n_speed;
        return;
    }

    // 2. 计算瞬时转向半径 (几何关系: R = L / tan(theta))
    float tan_theta = tanf(turn_angle_rad);
    float R_center = WHEELBASE_L / tan_theta;
    float R_center_abs = fabsf(R_center); // 半径取绝对值用于比例分配

    // 3. 计算左右后轮的转向半径
    float R_left, R_right;
    if (turn_angle > 0) {
        // 左拐: 左后轮是内侧, 右后轮是外侧
        R_left  = R_center_abs - (TRACK_B / 2.0f);
        R_right = R_center_abs + (TRACK_B / 2.0f);
    } else {
        // 右拐: 右后轮是内侧, 左后轮是外侧
        R_left  = R_center_abs + (TRACK_B / 2.0f);
        R_right = R_center_abs - (TRACK_B / 2.0f);
    }

    // 4. 按半径比例分配速度 (v = ω * r, 角速度ω相同)
    double ratio_left  = R_left  / R_center_abs;
    double ratio_right = R_right / R_center_abs;

    out_motor_speed->l_speed  = n_speed * ratio_left;
    out_motor_speed->r_speed = n_speed * ratio_right;

    // 5. 超速保护 (如果某一侧超过最大速度, 整体按比例压缩)
    float max_current_speed = fmaxf(fabsf(out_motor_speed->l_speed), fabsf(out_motor_speed->r_speed));
    if (max_current_speed > MAX_MOTOR_SPEED) {
        float scale_down = MAX_MOTOR_SPEED / max_current_speed;
        out_motor_speed->l_speed  *= scale_down;
        out_motor_speed->r_speed *= scale_down;
    }

}

//设置左轮速度 (已集成PID反馈控制)1前进，0后退
void set_l_speed(struct out_motor_speed motor_speed, int lmotor_dir)
{
    // 获取实际速度
    float actual_speed = get_left_motor_speed();
    
    // PID 计算
    float pid_output = pid_calculate(&left_pid, motor_speed.l_speed, actual_speed, MAIN_CONTROL_PERIOD);
    
    // 输出PWM (PID输出+前馈速度)
    float final_speed = motor_speed.l_speed + pid_output;
    int left_pwm = (int)((final_speed / MAX_MOTOR_SPEED) * lmotor_speed);
    
    // PWM 限制
    if(left_pwm > lmotor_speed) left_pwm = lmotor_speed;
    if(left_pwm < 0) left_pwm = 0;
    
    pwm_set_duty(ATOM0_CH0_P21_2, left_pwm); // 左轮PWM
    if(lmotor_dir==1)
    {
        gpio_high(P00_10);//高前进
    }
    else if(lmotor_dir==0)
    {
        gpio_low(P00_10);//低后退
    }

}
//设置右轮速度 (已集成PID反馈控制)1前进，0后退
void set_r_speed(struct out_motor_speed motor_speed, int rmotor_dir)
{
    // 获取实际速度
    float actual_speed = get_right_motor_speed();
    
    // PID 计算
    float pid_output = pid_calculate(&right_pid, motor_speed.r_speed, actual_speed, MAIN_CONTROL_PERIOD);
    
    // 输出PWM (PID输出+前馈速度)
    float final_speed = motor_speed.r_speed + pid_output;
    int right_pwm = (int)((final_speed / MAX_MOTOR_SPEED) * rmotor_speed);
    
    // PWM 限制
    if(right_pwm > rmotor_speed) right_pwm = rmotor_speed;
    if(right_pwm < 0) right_pwm = 0;
    
    pwm_set_duty(ATOM1_CH1_P21_3, right_pwm); // 右轮PWM
    if(rmotor_dir==1)
    {
        gpio_high(P00_11);//高前进
    }
    else if(rmotor_dir==0)
    {
        gpio_low(P00_11);//低后退
    }
}

//前轮转向
void front_wheel_turn(float turn_angle)
{

}

//**********路线拟合*********

/*
直线拟合,从start_index到start_index+1点
x:点的x坐标数组 y:点的y坐标数组 point_dis:每点间距（实际点间距会小） start_index:拟合点的起始索引
*/
void line_fit(double *x, double *y, int point_dis,int start_index)
{
    double dis=sqrt((x[start_index+1]-x[start_index])*(x[start_index+1]-x[start_index])+(y[start_index+1]-y[start_index])*(y[start_index+1]-y[start_index]));
    if(dis/point_dis>(int)(dis/point_dis)) 
    {
        fit_point_num = (int)(dis/point_dis) + 1;
    }

    if(fit_point_num > MAX_FIT_POINTS)
    {
        fit_point_num = MAX_FIT_POINTS;
    }

    // 在两点之间均匀插入fit_point_num个拟合点
    for(int i=0;i<fit_point_num;i++)
    {
        // 参数t从1/(fit_point_num+1)均匀到fit_point_num/(fit_point_num+1)
        gps_point_data.fit_x[i]=x[start_index]+(x[start_index+1]-x[start_index])*(i+1)/(fit_point_num+1);
        gps_point_data.fit_y[i]=y[start_index]+(y[start_index+1]-y[start_index])*(i+1)/(fit_point_num+1);
    }
}




/*
巡点导航
@param turn_angle 前轮转向角 (度, +左拐, -右拐)
@param n_speed 车身期望中心速度 (建议范围: 0 ~ MAX_MOTOR_SPEED)
@param dir 运动方向 (1=前进, -1=倒退)
*/
void runpoint(float n_speed, int dir)
{
    if(dir == 1)  // 前进
    {

        set_l_speed(motor_speed, 1);
        set_r_speed(motor_speed, 1);
    }
    else if(dir == -1)  // 倒退
    {
        set_l_speed(motor_speed, -1);
        set_r_speed(motor_speed, -1);
    }
}

/**
 * @brief 初始化 PID 控制器参数
 * 建议在程序启动时调用一次
 */
void pid_controller_init(void)
{
    pid_init(&left_pid, REAR_SPEED_PID_KP, REAR_SPEED_PID_KI, REAR_SPEED_PID_KD, MAX_MOTOR_SPEED);
    pid_init(&right_pid, REAR_SPEED_PID_KP, REAR_SPEED_PID_KI, REAR_SPEED_PID_KD, MAX_MOTOR_SPEED);
}

/**
 * @brief 重置 PID 控制器参数 (停止时调用)
 */
void pid_controller_reset(void)
{
    pid_reset(&left_pid);
    pid_reset(&right_pid);
}
