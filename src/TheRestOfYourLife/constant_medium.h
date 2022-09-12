//
// Created by George on 2022/8/24.
//

#ifndef RAY_TRACING_CONSTANT_MEDIUM_H
#define RAY_TRACING_CONSTANT_MEDIUM_H

#include "rtweekend.h"

#include "hittable.h"
#include "material.h"
#include "texture.h"

class constant_medium : public hittable {
public:
    constant_medium(shared_ptr<hittable> b, double d, shared_ptr<texture> a)
            : boundary(b), neg_inv_density(-1 / d), phase_function(make_shared<isotropic>(a)) {}

    constant_medium(shared_ptr<hittable> b, double d, Color c)
            : boundary(b), neg_inv_density(-1 / d), phase_function(make_shared<isotropic>(c)) {}

    virtual bool hit(const Ray &r, double t_min, double t_max, hit_record &rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override {
        return boundary->bounding_box(time0, time1, output_box);
    }

public:
    shared_ptr<hittable> boundary;
    shared_ptr<material> phase_function;
    double neg_inv_density;
};

bool constant_medium::hit(const Ray &r, double t_min, double t_max, hit_record &rec) const {

//    const bool enableDebug = false; // 偶尔打印一些样本，调试用
//    const bool debugging = enableDebug && random_double() < 0.00001;

    hit_record rec1, rec2;

    // 射线1 是否命中边界的包围盒 (获取前点)
    if (!boundary->hit(r, -infinity, infinity, rec1))
        return false;

    // 将 射线1 与边界包围盒的命中点作为 射线2 的起点 (获取后点)
    if (!boundary->hit(r, rec1.t + 0.0001, infinity, rec2))
        return false;

//    if (debugging) std::cerr << "\nt_min=" << rec.t << ", t_max=" << rec.t << "\n";

    if (rec1.t < t_min) rec1.t = t_min;
    if (rec2.t > t_max) rec2.t = t_max;

    if (rec1.t >= rec2.t)
        return false;

    if (rec1.t < 0)
        rec1.t = 0;

    // 获取光线在 volume 内的传播距离
    const auto ray_length = r.direction().length();
    const auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;

    // -密度倒数 * log(0~1) 随机
    const auto hit_distance = neg_inv_density * log(random_double());

    if (hit_distance > distance_inside_boundary)
        return false;

    // 设置 hit_record 距离和交点
    rec.t = rec1.t + hit_distance / ray_length;
    rec.p = r.at(rec.t);

//    if (debugging) {
//        std::cerr << "hit_distance = " << hit_distance << '\n'
//                  << "rec.t = " << rec.t << '\n'
//                  << "rec.p = " << rec.p << '\n';
//    }

    rec.normal = Vec3(1, 0, 0);     // 任意值，不参与计算
    rec.front_face = true;                      // 任意值，不参与计算
    rec.mat_ptr = phase_function;

    return true;
}

#endif //RAY_TRACING_CONSTANT_MEDIUM_H
