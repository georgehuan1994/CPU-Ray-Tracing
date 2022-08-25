#ifndef RAY_H
#define RAY_H

#include "../math/vec3.h"

class Ray {
public:
    Ray() = default;

    Ray(const Point3 &origin, const Vec3 &direction, double time = 0.0) : orig(origin), dir(direction), tm(time) {}

    Point3 origin() const { return orig; }

    Vec3 direction() const { return dir; }

    double time() const { return tm; }

    /// <summary>
    /// 交点
    /// </summary>
    /// <param name="t">距离</param>
    /// <returns>交点坐标</returns>
    Point3 at(double t) const {
        return orig + t * dir;
    }

public:
    Point3 orig;
    Vec3 dir;
    double tm;  // 光线所在的时刻
};

#endif