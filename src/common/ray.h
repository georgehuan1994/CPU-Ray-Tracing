#ifndef RAY_H
#define RAY_H

#include "../math/vec3.h"

class Ray 
{
public:
    Ray() {}
    Ray(const Point3& origin, const Vec3& direction): orig(origin), dir(direction) {}

    Point3 origin() const { return orig; }
    Vec3 direction() const { return dir; }

    /// <summary>
    /// 交点
    /// </summary>
    /// <param name="t">距离</param>
    /// <returns>交点坐标</returns>
    Point3 at(double t) const 
    {
        return orig + t * dir;
    }

public:
    Point3 orig;
    Vec3 dir;
};

#endif