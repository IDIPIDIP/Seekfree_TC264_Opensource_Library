#ifndef CODE_TINTEGRATE_H__
#define CODE_TINTEGRATE_H__
#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

#define INTEGRATE_MAX_POINTS (500)

// 融合输出缓存：
// 调用 integrate/integrate_with_noise 后，结果会写入此结构体。
// 下标范围为 [0, 返回值-1]。
struct integrate
{
    float x[INTEGRATE_MAX_POINTS]; // 融合后点的x坐标数组
    float y[INTEGRATE_MAX_POINTS]; // 融合后点的y坐标数组
};
extern struct integrate integrate_point;// 融合点全局实例

/**
 * @brief 使用默认噪声参数进行卡尔曼融合（GPS + IMU）。
 * @param x_gps GPS 曲线 x 数组（长度 n）
 * @param y_gps GPS 曲线 y 数组（长度 n）
 * @param x_imu IMU 曲线 x 数组（长度 n）
 * @param y_imu IMU 曲线 y 数组（长度 n）
 * @param n 融合输入点数（同索引一一对应），范围 1 ~ INTEGRATE_MAX_POINTS
 */
int integrate(const float *x_gps, const float *y_gps,const float *x_imu,
             const float *y_imu, int n);

/**
 * @brief 使用自定义噪声参数进行卡尔曼融合。
 * @param x_gps GPS 曲线 x 数组（长度 n）
 * @param y_gps GPS 曲线 y 数组（长度 n）
 * @param x_imu IMU 曲线 x 数组（长度 n）
 * @param y_imu IMU 曲线 y 数组（长度 n）
 * @param n 融合输入点数（同索引一一对应），范围 1 ~ INTEGRATE_MAX_POINTS
 * @param process_q 过程噪声 Q，越大越“跟随变化”，越小越“平滑”
 * @param gps_r GPS 测量噪声 R，越大表示越不信任 GPS
 * @param imu_r IMU 测量噪声 R，越大表示越不信任 IMU
 * @return 成功返回写入点数 n；失败返回 0
 * @note 建议：
 * process_q=0.001f, gps_r=0.5f, imu_r=1.5f
 */
int integrate_with_noise(const float *x_gps, const float *y_gps,const float *x_imu,
                         const float *y_imu, int n,float process_q, float gps_r, float imu_r);


#endif /* CODE_TINTEGRATE_H_ */
