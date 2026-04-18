#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

struct integrate integrate_point;// 融合点全局实例

/**
 * @brief 对单个标量通道做一次“GPS + IMU”顺序卡尔曼更新。
 * @param z_gps 当前点的 GPS 测量值
 * @param z_imu 当前点的 IMU 测量值
 * @param x_hat 输入/输出：当前通道的状态估计值
 * @param p 输入/输出：当前通道的估计协方差
 * @param process_q 过程噪声 Q
 * @param gps_r GPS 测量噪声 R
 * @param imu_r IMU 测量噪声 R
 * @return 当前点融合后的估计值
 */
static float kalman_fuse_scalar(float z_gps, float z_imu,float *x_hat, float *p,float process_q,
                                float gps_r, float imu_r)
{
	// 预测：常值模型
	float p_pred = *p + process_q;
	float x_pred = *x_hat;

	// 使用 GPS 更新
	float k_gps = p_pred / (p_pred + gps_r);
	float x_upd = x_pred + k_gps * (z_gps - x_pred);
	float p_upd = (1.0f - k_gps) * p_pred;

	// 使用 IMU 更新
	float k_imu = p_upd / (p_upd + imu_r);
	x_upd = x_upd + k_imu * (z_imu - x_upd);
	p_upd = (1.0f - k_imu) * p_upd;

	*x_hat = x_upd;
	*p = p_upd;
	return x_upd;
}

/**
 * @brief 融合两组等长曲线（GPS + IMU），并把结果写入全局 integrate_point。
 * @details
 * 1) x 和 y 分别作为两个独立通道进行融合；
 * 2) 每个索引 i 先预测，再顺序使用 GPS 与 IMU 更新；
 * 3) 不做插值或重采样，默认按“同索引点”一一对应融合。
 */
int integrate_with_noise(const float *x_gps, const float *y_gps,const float *x_imu, const float *y_imu, int n,
                        float process_q, float gps_r, float imu_r)
{
	// 输入安全检查：失败直接返回，不修改历史结果。
	if (!x_gps || !y_gps || !x_imu || !y_imu) return 0;
	if (n <= 0 || n > INTEGRATE_MAX_POINTS) return 0;

	if (process_q <= 0.0f || gps_r <= 0.0f || imu_r <= 0.0f) return 0;

	// 初值取两路测量均值，协方差初始为 1。
	float x_hat = 0.5f * (x_gps[0] + x_imu[0]);
	float y_hat = 0.5f * (y_gps[0] + y_imu[0]);
	float p_x = 1.0f;
	float p_y = 1.0f;

	integrate_point.x[0] = x_hat;
	integrate_point.y[0] = y_hat;

	for (int i = 1; i < n; i++)
	{
		integrate_point.x[i] = kalman_fuse_scalar(x_gps[i], x_imu[i],&x_hat, &p_x,process_q, gps_r, imu_r);
		integrate_point.y[i] = kalman_fuse_scalar(y_gps[i], y_imu[i],&y_hat, &p_y,process_q, gps_r, imu_r);
	}

	return n;
}

int integrate(const float *x_gps, const float *y_gps,
			  const float *x_imu, const float *y_imu, int n)
{
	// 默认参数偏向“GPS 更可信，IMU 次之，整体较平滑”。
	const float default_process_q = 0.001f;
	const float default_gps_r = 0.5f;
	const float default_imu_r = 1.5f;

	return integrate_with_noise(x_gps, y_gps, x_imu, y_imu, n,default_process_q, default_gps_r, default_imu_r);
}
