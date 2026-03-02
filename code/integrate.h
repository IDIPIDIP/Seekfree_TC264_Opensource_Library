#ifndef CODE_TINTEGRATE_H__
#define CODE_TINTEGRATE_H__
#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

struct integrate
{
    float x[MAX_FIT_POINTS]; // 融合后点的x坐标数组
    float y[MAX_FIT_POINTS]; // 融合后点的y坐标数组
    float fit_x[MAX_FIT_POINTS]; // 拟合点的x坐标数组
    float fit_y[MAX_FIT_POINTS]; // 拟合点的y坐标数组
    float current_x; // 当前点的x坐标
    float current_y; // 当前点的y坐标
};
extern struct integrate integrate_point;// 融合点全局实例

#endif /* CODE_TINTEGRATE_H_ */
