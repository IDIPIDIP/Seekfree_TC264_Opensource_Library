/*
 * imu.h
 *
 *  Created on: 2026年2月28日
 *      Author: 杨金乐
 */


#ifndef CODE_IMU_H_
#define CODE_IMU_H_


#define RtA         57.295779  //弧度->角度
#define AtR         0.0174533  //角度->弧度

typedef struct
{
    float Xdata;        //零漂参数X
    float Ydata;        //零漂参数y
    float Zdata;        //零漂参数z
}IMU_zero_offest_t;


typedef struct 
{
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z; 
    float mag_x;
    float mag_y;
    float mag_z;
}IMU_param_t;

extern IMU_param_t IMU_Data;          //陀螺仪数据结构体
extern IMU_zero_offest_t Gyro_Offest;      //陀螺仪零漂结构体（最简单的方法得到的零漂）
extern IMU_zero_offest_t Acc_Offest;       //加速度计零漂结构体
extern IMU_zero_offest_t Mag_Offest;       //磁力计零漂结构体
extern uint8 imu_init_flag;         //陀螺仪去除零漂标志位 ， 为1表示去零漂完成
extern float Angle_Z_Quaternions,Angle_Z_Quaternions_true;   //用笛卡尔坐标系下的欧拉角Z轴做偏航角

void IMU_Init(void);
void IMU_Get_Data(void);
void IMU_Update_Attitude(void);
void IMU_offest_Init(void);
void IMU_Offest_Compensate(IMU_param_t *Data,IMU_zero_offest_t *Gyro_Offest,IMU_zero_offest_t *Acc_Offest,IMU_zero_offest_t *Mag_Offest);
float IMU_Get_Yaw_TrueNorth(void);
void IMU_Set_Magnetic_Declination(float declination_deg);
void MahonyAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az,float mx,float my,float mz);
float invSqrt(float x);

#endif
