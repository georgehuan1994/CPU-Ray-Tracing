//
// Created by George on 2022/8/22.
// Axis-aligned bounding box class.
//

#ifndef RAY_TRACING_AABB_H
#define RAY_TRACING_AABB_H

#include "rtweekend.h"

class aabb {
public:
    aabb() = default;

    // 使用两个点构造 AABB
    aabb(const Point3 &a, const Point3 &b) {
        minimum = a;
        maximum = b;
    }

    Point3 min() const { return minimum; }

    Point3 max() const { return maximum; }

    // 传入射线、起点距离、最大距离，判断是射线否与 AABB 相交
    bool hit(const Ray &r, double t_min, double t_max) const {

//        检查射线在三个维度上的重叠情况
//        for (int i = 0; i < 3; i++) {
//            auto t0 = fmin(
//                    (minimum[i] - r.origin()[i]) / r.direction()[i],
//                    (maximum[i] - r.origin()[i]) / r.direction()[i]);
//            auto t1 = fmax(
//                    (minimum[i] - r.origin()[i]) / r.direction()[i],
//                    (maximum[i] - r.origin()[i]) / r.direction()[i]);
//            t_min = fmax(t0, t_min);
//            t_max = fmin(t1, t_max);
//            if (t_max <= t_min)
//                return false;
//        }
//        return true;

        // 重叠检查优化版本
        for (int a = 0; a < 3; a++) {
            auto invD = 1.0f / r.direction()[a];
            auto t0 = (min()[a] - r.origin()[a]) * invD;
            auto t1 = (max()[a] - r.origin()[a]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min)
                return false;
        }
        return true;
    }

    Point3 minimum;
    Point3 maximum;
};

aabb surrounding_box(aabb box0, aabb box1) {
    Point3 small(
            fmin(box0.min().x(), box1.min().x()),
            fmin(box0.min().y(), box1.min().y()),
            fmin(box0.min().z(), box1.min().z()));
    Point3 big(
            fmax(box0.max().x(), box1.max().x()),
            fmax(box0.max().y(), box1.max().y()),
            fmax(box0.max().z(), box1.max().z()));
    return aabb(small, big);
}

#endif //RAY_TRACING_AABB_H
