#ifndef CODE_FITTING_H_
#define CODE_FITTING_H_

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"
extern int point_num;//拟合点数
typedef struct
{
	float alpha;     // Catmull-Rom 参数化系数: 0.0=uniform, 0.5=centripetal(推荐), 1.0=chordal
	int min_sub;     // 每段最小细分数。越大越平滑, 但计算量更高
	int max_sub;     // 每段最大细分数。用于限制极端情况下的计算量
	float sub_scale; // 控制曲线生成多少个插值点，数值越大曲线越平滑、密度越高；数值越小越稀疏
} curve_fit_cfg_t;

/**
 * @brief 质量优先的曲线拟合接口（centripetal Catmull-Rom + 近似弧长等距重采样）
 *
 * @param px      输入点 x 数组
 * @param py      输入点 y 数组
 * @param n       输入点数量, 要求 n >= 2
 * @param ds      输出点目标间距(单位同坐标), 要求 ds > 0
 * @param tmp_x   输出点 x 数组
 * @param tmp_y   输出点 y 数组
 * @param tmp_cap 输出数组容量（tmp_x/tmp_y 实际长度必须都 >= tmp_cap）
 * @param cfg     算法配置。传 NULL 时使用默认参数：alpha=0.5, min_sub=16, max_sub=96, sub_scale=0.25
 * @return 成功时返回输出点数量(>=1)，失败返回0 若容量不足，会返回当前已写入点数（<= tmp_cap）并提前结束。
 */
int curve_fit_ex(const float *px, const float *py, int n, float ds,
				 float *tmp_x, float *tmp_y, int tmp_cap,
				 const curve_fit_cfg_t *cfg);

/**
 * @brief 默认配置适合大多数场景，若需要更强平滑或更低计算量，建议改用 curve_fit_ex。
 */
int curve_fit(const float *px, const float *py, int n, float ds, float *tmp_x, float *tmp_y, int tmp_cap);

/*
 * 调用示例:
 *   float out_x[200], out_y[200];
 *   int out_n = curve_fit(gps_x, gps_y, point_num, 0.10f, out_x, out_y, 200);
 *
 * 或自定义参数:
 *
 *   curve_fit_cfg_t cfg = {0.5f, 20, 120, 0.22f};
 *   int out_n = curve_fit_ex(gps_x, gps_y, point_num, 0.10f, out_x, out_y, 200, &cfg);
 */
#endif /* CODE_FITTING_H_ */
