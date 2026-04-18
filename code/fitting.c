#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

int point_num=0;//ÄâºÏµãÊý
static float clampf(float x, float lo, float hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static int clampi(int x, int lo, int hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static float dist2_xy(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

static float knot_increment(float d2, float alpha)
{
    const float eps2 = 1e-8f;
    if (d2 < eps2) d2 = eps2;

    if (alpha <= 0.0f) {
        return 1.0f;
    } else if (alpha >= 1.0f) {
        return sqrtf(d2);
    }

    // |Pj-Pi|^alpha = (d2)^(alpha/2)
    return powf(d2, 0.5f * alpha);
}

static float lerp_by_time(float p0, float p1, float t0, float t1, float t)
{
    const float eps = 1e-6f;
    float den = t1 - t0;
    if (den < 0.0f) den = -den;
    if (den < eps) return p1;
    return ((t1 - t) / (t1 - t0)) * p0 + ((t - t0) / (t1 - t0)) * p1;
}

static void catmull_nonuniform_xy(float p0x, float p0y,
                                  float p1x, float p1y,
                                  float p2x, float p2y,
                                  float p3x, float p3y,
                                  float alpha, float u,
                                  float *x, float *y)
{
    float t0 = 0.0f;
    float t1 = t0 + knot_increment(dist2_xy(p0x, p0y, p1x, p1y), alpha);
    float t2 = t1 + knot_increment(dist2_xy(p1x, p1y, p2x, p2y), alpha);
    float t3 = t2 + knot_increment(dist2_xy(p2x, p2y, p3x, p3y), alpha);

    float s = t1 + (t2 - t1) * clampf(u, 0.0f, 1.0f);

    float a1x = lerp_by_time(p0x, p1x, t0, t1, s);
    float a1y = lerp_by_time(p0y, p1y, t0, t1, s);
    float a2x = lerp_by_time(p1x, p2x, t1, t2, s);
    float a2y = lerp_by_time(p1y, p2y, t1, t2, s);
    float a3x = lerp_by_time(p2x, p3x, t2, t3, s);
    float a3y = lerp_by_time(p2y, p3y, t2, t3, s);

    float b1x = lerp_by_time(a1x, a2x, t0, t2, s);
    float b1y = lerp_by_time(a1y, a2y, t0, t2, s);
    float b2x = lerp_by_time(a2x, a3x, t1, t3, s);
    float b2y = lerp_by_time(a2y, a3y, t1, t3, s);

    *x = lerp_by_time(b1x, b2x, t1, t2, s);
    *y = lerp_by_time(b1y, b2y, t1, t2, s);
}

static int push_point(float x, float y, float *out_x, float *out_y, int out_count, int out_cap)
{
    if (out_count >= out_cap) return out_count;
    out_x[out_count] = x;
    out_y[out_count] = y;
    return out_count + 1;
}

int curve_fit_ex(const float *px, const float *py, int n, float ds,
                 float *tmp_x, float *tmp_y, int tmp_cap,
                 const curve_fit_cfg_t *cfg)
{
    if (!px || !py || !tmp_x || !tmp_y || n < 2 || ds <= 0.0f || tmp_cap <= 0) return 0;

    curve_fit_cfg_t local_cfg;
    if (cfg) {
        local_cfg = *cfg;
    } else {
        local_cfg.alpha = 0.5f;
        local_cfg.min_sub = 16;
        local_cfg.max_sub = 96;
        local_cfg.sub_scale = 0.25f;
    }

    local_cfg.alpha = clampf(local_cfg.alpha, 0.0f, 1.0f);
    if (local_cfg.sub_scale < 0.05f) local_cfg.sub_scale = 0.05f;
    local_cfg.min_sub = clampi(local_cfg.min_sub, 4, 1024);
    local_cfg.max_sub = clampi(local_cfg.max_sub, local_cfg.min_sub, 2048);

    int out_count = 0;
    out_count = push_point(px[0], py[0], tmp_x, tmp_y, out_count, tmp_cap);
    if (out_count >= tmp_cap) return out_count;

    float dist_to_next = ds;
    const float eps = 1e-6f;

    for (int i = 0; i < n - 1; i++) {
        int i0 = (i == 0)     ? 0     : i - 1;
        int i1 = i;
        int i2 = i + 1;
        int i3 = (i + 2 >= n) ? n - 1 : i + 2;

        float seg_dx = px[i2] - px[i1];
        float seg_dy = py[i2] - py[i1];
        float seg_len = sqrtf(seg_dx*seg_dx + seg_dy*seg_dy);

        int sub = (int)(seg_len / (ds * local_cfg.sub_scale)) + 2;
        sub = clampi(sub, local_cfg.min_sub, local_cfg.max_sub);

        float prev_x = px[i1];
        float prev_y = py[i1];

        for (int k = 1; k <= sub; k++) {
            float u = (float)k / (float)sub;
            float cur_x, cur_y;
            catmull_nonuniform_xy(px[i0], py[i0], px[i1], py[i1], px[i2], py[i2], px[i3], py[i3],
                                  local_cfg.alpha, u, &cur_x, &cur_y);

            float dx = cur_x - prev_x;
            float dy = cur_y - prev_y;
            float seg = sqrtf(dx*dx + dy*dy);

            while (seg >= dist_to_next && out_count < tmp_cap) {
                float r = dist_to_next / seg;
                float nx = prev_x + r * dx;
                float ny = prev_y + r * dy;

                out_count = push_point(nx, ny, tmp_x, tmp_y, out_count, tmp_cap);
                if (out_count >= tmp_cap) return out_count;

                prev_x = nx;
                prev_y = ny;
                dx = cur_x - prev_x;
                dy = cur_y - prev_y;
                seg = sqrtf(dx*dx + dy*dy);
                dist_to_next = ds;
            }

            if (seg > eps) {
                dist_to_next -= seg;
                if (dist_to_next < eps) dist_to_next = ds;
            }

            prev_x = cur_x;
            prev_y = cur_y;
        }
    }

    if (out_count > 0) {
        float end_x = px[n - 1];
        float end_y = py[n - 1];
        float d2 = dist2_xy(tmp_x[out_count - 1], tmp_y[out_count - 1], end_x, end_y);
        if (d2 > 1e-10f) {
            out_count = push_point(end_x, end_y, tmp_x, tmp_y, out_count, tmp_cap);
        } else {
            tmp_x[out_count - 1] = end_x;
            tmp_y[out_count - 1] = end_y;
        }
    }

    return out_count;
}

int curve_fit(const float *px, const float *py, int n, float ds, float *tmp_x, float *tmp_y, int tmp_cap)
{
    curve_fit_cfg_t cfg;
    cfg.alpha = 0.5f;
    cfg.min_sub = 16;
    cfg.max_sub = 96;
    cfg.sub_scale = 0.25f;
    return curve_fit_ex(px, py, n, ds, tmp_x, tmp_y, tmp_cap, &cfg);
}
