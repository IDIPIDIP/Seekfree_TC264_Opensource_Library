/*
 * int.c
 *
 *  Created on: 2026年2月27日
 *      Author: IDIPIDIP
 */

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"



int L_PWM_P=50;//左电机PWM频率
int L_PWM_K=5000;//左电机PWM占空比 =PWM_K/10000
int R_PWM_P=50;//右电机PWM频率
int R_PWM_K=5000;//右电机PWM占空比 =PWM_K/10000
int l_encoder;//左后编码器计数值
int r_encoder;//右后编码器计数值
int f_encoder;//前轮编码器计数值


void init()
{
    pwm_init(ATOM0_CH0_P21_2,L_PWM_P,L_PWM_K);
    pwm_init(ATOM0_CH1_P21_3,R_PWM_P,R_PWM_K);
    pwm_init(speaker_pwm_channel, 500, 0); // 初始化扬声器PWM
    encoder_init();
    flash_init();
    encoder_clear_count(TIM2_ENCODER);
    encoder_clear_count(TIM3_ENCODER);
    encoder_clear_count(TIM4_ENCODER);
    pit_ms_init(CCU60_CH0, 5);//定时器0，5ms中断一次，用于获取编码器和按键扫描
    gnss_init(TAU1201);
    tft180_init();
    tft180_set_font(TFT180_6X8_FONT);
    key_init(10);//按键扫描周期10ms
    scc8660_init();//摄像头初始化
    

}
