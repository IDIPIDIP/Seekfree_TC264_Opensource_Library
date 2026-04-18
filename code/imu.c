/*
 * imu.c
 *
 *  Created on: 2026年2月28日
 *      Author: 杨金乐
 */


/*********************************************************************************************************************
*功能名称   IMU姿态控制 获取偏航角
*引脚定义   SPI通信： CLK:P20_11  CS:P20_13 MOSI:P20_14 MISO:P20_12
*引脚定义   IIC通信： SCL:P10_9 SDA:P10_10
*
*硬件参数 支持SPI 和IIC双通信
*
*
*      IIC通信
*
*********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "imu.h"

soft_iic_info_struct IMU_soft_iic_InitStructure;


IMU_param_t IMU_Data;          //陀螺仪数据结构体
IMU_zero_offest_t Gyro_Offest;      //陀螺仪零漂结构体（最简单的方法得到的零漂）
IMU_zero_offest_t Acc_Offest;       //加速度零漂计结构体
IMU_zero_offest_t Mag_Offest;       //磁力计零漂结构体
uint8 imu_init_flag;         //陀螺仪去除零漂标志位 ， 为1表示去零漂完成
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      IMU初始化  开启中断获取IMU实时数据
//  参数说明
//  参数说明
//  返回参数      void
//  使用示例      更新速度为200Hz
//-------------------------------------------------------------------------------------------------------------------
void IMU_Init(void)
{
    imu_init_flag=imu963ra_init();
    IMU_offest_Init();
    if (imu_init_flag==0) pit_ms_init(CCU60_CH0,5);

}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      周期性获取IMU数据 5ms获取一次陀螺仪数据
//  参数说明      储存在结构体中 IMU_Data
//  参数说明        量程分别为 ±2G ±1000dps 2G
//  返回参数      void
//  使用示例      更新速度为1000Hz
//-------------------------------------------------------------------------------------------------------------------

void IMU_Get_Data(void)
{
    imu963ra_get_acc();
    imu963ra_get_gyro();
    imu963ra_get_mag();
    IMU_Data.accel_x=imu963ra_acc_transition(imu963ra_acc_x);
    IMU_Data.accel_y=imu963ra_acc_transition(imu963ra_acc_y);
    IMU_Data.accel_z=imu963ra_acc_transition(imu963ra_acc_z);
    IMU_Data.gyro_x=imu963ra_gyro_transition(imu963ra_gyro_x);
    IMU_Data.gyro_y=imu963ra_gyro_transition(imu963ra_gyro_y);
    IMU_Data.gyro_z=imu963ra_gyro_transition(imu963ra_gyro_z);
    IMU_Data.mag_x=imu963ra_mag_transition(imu963ra_mag_x);
    IMU_Data.mag_y=imu963ra_mag_transition(imu963ra_mag_y);
    IMU_Data.mag_z=imu963ra_mag_transition(imu963ra_mag_z);
}   

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      获取零漂 循环200次获取陀螺仪数据求平均值作为零漂
//  参数说明      储存在结构体中 Gyro_Offest  Acc_Offest
//  参数说明
//  返回参数      void
//  使用示例      更新速度为200Hz
//-------------------------------------------------------------------------------------------------------------------
void IMU_offest_Init(void)
{
    Gyro_Offest.Xdata = 0;
    Gyro_Offest.Ydata = 0;
    Gyro_Offest.Zdata = 0;
    Acc_Offest.Xdata=0;
    Acc_Offest.Ydata=0;
    Acc_Offest.Zdata=0;
    Mag_Offest.Xdata=0;
    Mag_Offest.Ydata=0;
    Mag_Offest.Zdata=0;
    for(int i = 0; i < 200 ; i++)
    {
        IMU_Get_Data();
        Gyro_Offest.Xdata += IMU_Data.gyro_x;
        Gyro_Offest.Ydata += IMU_Data.gyro_y;
        Gyro_Offest.Zdata += IMU_Data.gyro_z;
        Acc_Offest.Xdata += IMU_Data.accel_x;
        Acc_Offest.Ydata += IMU_Data.accel_y;
        Acc_Offest.Zdata += IMU_Data.accel_z;
        Mag_Offest.Xdata += IMU_Data.mag_x;
        Mag_Offest.Ydata += IMU_Data.mag_y;
        Mag_Offest.Zdata += IMU_Data.mag_z;
        system_delay_ms(5);
    }
    Acc_Offest.Xdata /= 200.0;
    Acc_Offest.Ydata /= 200.0;
    Acc_Offest.Zdata /= 200.0;
    Gyro_Offest.Xdata /= 200.0;
    Gyro_Offest.Ydata /= 200.0;
    Gyro_Offest.Zdata /= 200.0;
    Mag_Offest.Xdata /= 200.0;
    Mag_Offest.Ydata /= 200.0;
    Mag_Offest.Zdata /= 200.0;
    
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      零漂补偿 将零漂附近的值取为零漂值，这样以来静止状态下的陀螺仪则可处理为0
//  参数说明      储存在结构体中 Gyro_Offest  Acc_Offest
//  参数说明
//  返回参数      void
//  使用示例      更新速度为200Hz
//-------------------------------------------------------------------------------------------------------------------
void IMU_Offest_Compensate(IMU_param_t *Data,IMU_zero_offest_t *Gyro_Offest,IMU_zero_offest_t *Acc_Offest,IMU_zero_offest_t *Mag_Offest)
{
    float min_limit=3;//零漂范围内则取零漂值，否则取原值
    IMU_Get_Data();
    //陀螺仪获取到数据与零漂做差
    if(Data->gyro_x - Gyro_Offest->Xdata <min_limit && Data->gyro_x - Gyro_Offest->Xdata > -min_limit)
    {
        Data->gyro_x = Gyro_Offest->Xdata;
    }
    if(Data->gyro_y - Gyro_Offest->Ydata <min_limit && Data->gyro_y - Gyro_Offest->Ydata > -min_limit)
    {
        Data->gyro_y = Gyro_Offest->Ydata;
    }
    if(Data->gyro_z - Gyro_Offest->Zdata <min_limit && Data->gyro_z - Gyro_Offest->Zdata > -min_limit)
    {
        Data->gyro_z = Gyro_Offest->Zdata;
    }
    if(Data->accel_x - Acc_Offest->Xdata < min_limit && Data->accel_x - Acc_Offest->Xdata > -min_limit)
    {
        Data->accel_x = Acc_Offest->Xdata;
    }
    if(Data->accel_y - Acc_Offest->Ydata < min_limit && Data->accel_y - Acc_Offest->Ydata > -min_limit)
    {
        Data->accel_y = Acc_Offest->Ydata;
    }
    if(Data->accel_z - Acc_Offest->Zdata < min_limit && Data->accel_z - Acc_Offest->Zdata > -min_limit)
    {
        Data->accel_z = Acc_Offest->Zdata;
    }
    if(Data->mag_x - Mag_Offest->Xdata < min_limit && Data->mag_x - Mag_Offest->Xdata > -min_limit)
    {
        Data->mag_x = Mag_Offest->Xdata;
    }
    if(Data->mag_y - Mag_Offest->Ydata < min_limit && Data->mag_y - Mag_Offest->Ydata > -min_limit)
    {
        Data->mag_y = Mag_Offest->Ydata;
    }
    if(Data->mag_z - Mag_Offest->Zdata < min_limit && Data->mag_z - Mag_Offest->Zdata > -min_limit)
    {
        Data->mag_z = Mag_Offest->Zdata;
    }

}

float sampleFreq=200.0f;          // 200Hz采样频率
volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f; //四元数初始值
volatile float integralFBx = 0.0f, integralFBy = 0.0f, integralFBz = 0.0f; //积分误差
volatile float twoKp1= 2.0f,twoKi1=0.1f; //加速度计修正的PI参数 2*Kp比例增益 2*Ki积分增益
volatile float twoKp2= 2.0f,twoKi2=0.1f; //磁力计修正的PI参数 2*Kp比例增益 2*Ki积分增益
float Angle_Z_Quaternions=0,Angle_Z_Quaternions_true=0;   //用笛卡尔坐标系下的欧拉角Z轴做偏航角
static float imu_magnetic_declination_deg = 0.0f;          // 磁偏角 东偏为正 西偏为负

static float imu_normalize_angle_360(float angle_deg)
{
    while(angle_deg >= 360.0f)
    {
        angle_deg -= 360.0f;
    }
    while(angle_deg < 0.0f)
    {
        angle_deg += 360.0f;
    }
    return angle_deg;
}

void MahonyAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
{
    float recipNorm;//reciprocal of the norm  
    float q0q0,q1q1,q2q2,q3q3,q0q1,q0q2,q0q3,q1q2,q1q3,q2q3;
    float q0_pre,q1_pre,q2_pre;
    float halfvx, halfvy, halfvz;//二分之一的估计重力方向
    float halfex_a, halfey_a, halfez_a;//二分之一的误差
    float halfex_m, halfey_m, halfez_m;//二分之一的误差
    
    float hx,hy,bx,bz; //h:真实磁场强度 只测xy方向的值 b:校正后的磁场强度 只留x方向的值
    float halfwx, halfwy, halfwz; //二分之一的校正后的磁场强度
    q0q0=q0 * q0;
    q1q1=q1 * q1;
    q2q2=q2 * q2;
    q3q3=q3 * q3;
    q0q1=q0 * q1;
    q0q2=q0 * q2;
    q0q3=q0 * q3;
    q1q2=q1 * q2;
    q1q3=q1 * q3;
    q2q3=q2 * q3;

    //归一化加速度计测量值
    recipNorm=invSqrt(ax * ax + ay * ay + az * az);
    ax*=recipNorm;
    ay*=recipNorm;
    az*=recipNorm;
    //估计重力方向
    halfvx=q1q3-q0q2;
    halfvy=q0q1+q2q3;
    halfvz=q0q3+q1q2;
    //估计与实际的叉乘误差
    halfex_a=ay * halfvz - az * halfvy;
    halfey_a=az * halfvx - ax * halfvz;
    halfez_a=ax * halfvy - ay * halfvx;

    //归一化磁力计数据
    recipNorm=invSqrt(mx * mx + my * my + mz * mz);
    mx*=recipNorm;
    my*=recipNorm;
    mz*=recipNorm;
    //获取地球标准磁场强度和方向
    hx=2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
    hy=2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
    bx=sqrt(hx * hx + hy * hy);
    bz=2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));
    //将磁场强度转换到机体坐标系下
    halfwx=bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
    halfwy=bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
    halfwz=bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);
    //叉乘得到磁轴误差
    halfex_m=my * halfwz - mz * halfwy;
    halfey_m=mz * halfwx - mx * halfwz;
    halfez_m=mx * halfwy - my * halfwx;


    //积分误差
    if(twoKi1>0.0f)
    {
        integralFBx+=halfex_a*twoKi1*(1.0f/sampleFreq);
        integralFBy+=halfey_a*twoKi1*(1.0f/sampleFreq);
        integralFBz+=halfez_a*twoKi1*(1.0f/sampleFreq); 
    }
    if(twoKi2>0.0f)
    {
        integralFBx+=halfex_m*twoKi2*(1.0f/sampleFreq);
        integralFBy+=halfey_m*twoKi2*(1.0f/sampleFreq);
        integralFBz+=halfez_m*twoKi2*(1.0f/sampleFreq);    
    }
    if(twoKi1<=0.0f && twoKi2<=0.0f)
    {
        integralFBx=0.0f;
        integralFBy=0.0f;
        integralFBz=0.0f; 
    }
    gx+=integralFBx;
    gy+=integralFBy;
    gz+=integralFBz;
    //比例误差 积分误差和比例误差相加后得到新的陀螺仪测量值
    gx+=twoKp1*halfex_a+twoKp2*halfex_m;
    gy+=twoKp1*halfey_a+twoKp2*halfey_m;
    gz+=twoKp1*halfez_a+twoKp2*halfez_m;

    //积分陀螺仪测量值得到四元数微分方程的解
    gx*=0.5f*(1.0f/sampleFreq);
    gy*=0.5f*(1.0f/sampleFreq); 
    gz*=0.5f*(1.0f/sampleFreq);

    q0_pre=q0;
    q1_pre=q1;
    q2_pre=q2;
    q0+=-q1*gx-q2*gy-q3*gz;
    q1 += q0_pre*gx + q2_pre*gz - q3*gy;
    q2 += q0_pre*gy - q1_pre*gz + q3*gx;
    q3 += q0_pre*gz + q1_pre*gy - q2_pre*gx;
    //归一化四元数
    recipNorm=invSqrt(q0*q0+q1*q1+q2*q2+q3*q3);
    q0*=recipNorm;  
    q1*=recipNorm;
    q2*=recipNorm;
    q3*=recipNorm;
    Angle_Z_Quaternions_true=-atan2(2*q1*q2+2*q0*q3,-q3*q3-q2*q2+q0*q0+q1*q1)*RtA;//四元数获取偏航角
    //偏航角为 0-360度
    if(Angle_Z_Quaternions_true<0)
        Angle_Z_Quaternions_true+=360.0f;
    //只有大于阈值时才进行更新 阈值设置为1度 这样以来可以滤除小范围的抖动
    if (Angle_Z_Quaternions_true - Angle_Z_Quaternions > 1 || Angle_Z_Quaternions_true - Angle_Z_Quaternions < -1)
    {
        Angle_Z_Quaternions = Angle_Z_Quaternions_true;
    }
    
}

void IMU_Update_Attitude(void)
{
    float gx;
    float gy;
    float gz;
    float ax;
    float ay;
    float az;
    float mx;
    float my;
    float mz;

    if(imu_init_flag != 0)
    {
        return;
    }

    gx = (IMU_Data.gyro_x - Gyro_Offest.Xdata) * AtR;
    gy = (IMU_Data.gyro_y - Gyro_Offest.Ydata) * AtR;
    gz = (IMU_Data.gyro_z - Gyro_Offest.Zdata) * AtR;
    ax = IMU_Data.accel_x;
    ay = IMU_Data.accel_y;
    az = IMU_Data.accel_z;
    mx = IMU_Data.mag_x;
    my = IMU_Data.mag_y;
    mz = IMU_Data.mag_z;

    if((ax == 0.0f && ay == 0.0f && az == 0.0f) || (mx == 0.0f && my == 0.0f && mz == 0.0f))
    {
        return;
    }

    MahonyAHRSupdate(gx, gy, gz, ax, ay, az, mx, my, mz);
}

float IMU_Get_Yaw_TrueNorth(void)
{
    return imu_normalize_angle_360(Angle_Z_Quaternions + imu_magnetic_declination_deg);
}

void IMU_Set_Magnetic_Declination(float declination_deg)
{
    imu_magnetic_declination_deg = declination_deg;
}

float invSqrt(float x)
{
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i>>1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}
