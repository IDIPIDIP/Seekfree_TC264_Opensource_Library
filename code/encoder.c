#include "zf_common_headfile.h"

/*********************************************************************************************************************
*功能名称   编码器测 后轮转速 前轮转角
*引脚定义   转向    DIR:P00_9 LSB:P02_8
*引脚定义   左后轮  DIR:P00_8 LSB:P33_7
*引脚定义   右后轮  DIR:P02_7 LSB:P02_6
*硬件参数
*使用说明      前轮转角   encoder_steering           单位 弧度
*使用说明      左后轮转速  encoder_left_wheel_speed   单位 m/s
*使用说明      右后轮转速  encoder_right_wheel_speed  单位 m/s
*
*
*
*********************************************************************************************************************/
// ENCODER_STEERING_DIR_PIN     TIM4_ENCODER_CH2_P00_9
// ENCODER_STEERING_LSB_PIN     TIM4_ENCODER_CH1_P02_8
// ENCODER_LEFT_WHEEL_DIR_PIN   TIM2_ENCODER_CH2_P00_8
// ENCODER_LEFT_WHEEL_LSB_PIN   TIM2_ENCODER_CH1_P33_7
// ENCODER_RIGHT_WHEEL_DIR_PIN  TIM3_ENCODER_CH2_P02_7
// ENCODER_RIGHT_WHEEL_LSB_PIN  TIM3_ENCODER_CH1_P02_6
int16 encoder_pit_frequency= 200;//5ms 200Hz刷新频率
int16 encoder_steering_pulse_count = 0; // 前轮转向编码器脉冲计数
int16 encoder_left_wheel_5ms_speed = 0; // 左后轮编码器脉冲计数
int16 encoder_right_wheel_5ms_speed = 0; // 右后轮编码器脉冲计数

float encoder_steering=0;
float encoder_left_wheel_speed=0;
float encoder_right_wheel_speed=0;
float encoder_left_wheel_acceleration=0; // 左后轮瞬时加速度
float encoder_right_wheel_acceleration=0; // 右后轮瞬时加速度

uint8 ecoder_state_flag=1; //编码器状态标志位 1未开启 0开启

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      编码器初始化函数  测量前轮转向角  后轮转速
//  参数说明
//  参数说明
//  使用说明      前轮转角   encoder_steering           单位 弧度
//  使用说明      左后轮转速  encoder_left_wheel_speed   单位 m/s
//  使用说明      右后轮转速  encoder_right_wheel_speed  单位 m/s
//  返回参数      void
//  使用示例      更新速度为200Hz
//-------------------------------------------------------------------------------------------------------------------
void encoder_init(void)
{
    gpio_init(P02_8,GPI,1,GPI_PULL_UP);
    gpio_init(P00_9,GPI,1,GPI_PULL_UP);
    gpio_init(P33_7,GPI,1,GPI_PULL_UP);
    gpio_init(P00_8,GPI,1,GPI_PULL_UP);
    gpio_init(P02_6,GPI,1,GPI_PULL_UP);
    gpio_init(P02_7,GPI,1,GPI_PULL_UP);
    //配置编码器引脚 并启动编码器
    encoder_quad_init(TIM4_ENCODER,TIM4_ENCODER_CH1_P02_8,TIM4_ENCODER_CH2_P00_9);
    encoder_dir_init(TIM2_ENCODER,TIM2_ENCODER_CH1_P33_7,TIM2_ENCODER_CH2_P00_8);
    encoder_dir_init(TIM3_ENCODER,TIM3_ENCODER_CH1_P02_6,TIM3_ENCODER_CH2_P02_7);

    //开启pit定时中断 定时读取编码器计数值 和imu中断在一起 5ms获取一次数据
    ecoder_state_flag=0;
}

void encoder_loop(void)
{
    if(ecoder_state_flag==0)
    {
    //前轮转角和后轮转速部分
    static float encoder_left_wheel_speed_last=0;
    static float encoder_right_wheel_speed_last=0;
    encoder_steering_pulse_count=encoder_get_count(TIM4_ENCODER);//前轮转角脉冲计数
    encoder_left_wheel_5ms_speed=ENCODER_GET_WHEEL_COEFF(encoder_get_count(TIM2_ENCODER))*encoder_pit_frequency;//左后轮转速 弧度/s
    encoder_clear_count(TIM2_ENCODER);
    encoder_right_wheel_5ms_speed=ENCODER_GET_WHEEL_COEFF(encoder_get_count(TIM3_ENCODER))*encoder_pit_frequency;//右后轮转速 弧度/s
    encoder_clear_count(TIM3_ENCODER);
    encoder_steering=ENCODER_GET_STEER_ANGLE(encoder_steering_pulse_count);//前轮转角 角度
    //后轮加速度部分
    encoder_left_wheel_acceleration=(encoder_left_wheel_speed-encoder_left_wheel_speed_last)*encoder_pit_frequency;//左后轮加速度 m/s^2
    encoder_right_wheel_acceleration=(encoder_right_wheel_speed-encoder_right_wheel_speed_last)*encoder_pit_frequency;//右后轮加速度 m/s^2
    //后轮线速度部分
    encoder_left_wheel_speed=encoder_left_wheel_5ms_speed*ENCODER_WHEEL_RADIUS;//左后轮线速度 m/s
    encoder_right_wheel_speed=encoder_right_wheel_5ms_speed*ENCODER_WHEEL_RADIUS;//右后轮线速度 m/s
    
    encoder_left_wheel_speed_last=encoder_left_wheel_speed;
    encoder_right_wheel_speed_last=encoder_right_wheel_speed;
    }
}