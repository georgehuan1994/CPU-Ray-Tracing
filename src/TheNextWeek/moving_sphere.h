//
// Created by George on 2022/8/21.
//

#ifndef RAY_TRACING_MOVING_SPHERE_H
#define RAY_TRACING_MOVING_SPHERE_H

#include "../common//rtweekend.h"
#include "hittable.h"

class moving_sphere : public hittable {
public:
    moving_sphere() = default;

    moving_sphere(Point3 cen0, Point3 cen1, double _time0, double _time1, double r, shared_ptr<material> m) :
            center0(cen0), center1(cen1), time0(_time0), time1(_time1), radius(r), mat_ptr(m) {};

    virtual bool hit(const Ray &r, double t_min, double t_max, hit_record &rec) const override;

    Point3 center(double time) const;

public:
    Point3 center0, center1;
    double time0, time1;
    double radius;
    shared_ptr<material> mat_ptr;
};

Point3 moving_sphere::center(double time) const {
    return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

bool moving_sphere::hit(const Ray &r, double t_min, double t_max, hit_record &rec) const {
    Vec3 oc = r.origin() - center(r.time());              // 射线起点到球体中心

    // 简化的求根公式
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;

    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // 找到距离最近的根，并判断是否在可接受的范围内：[0.001, infinity]
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root)
    {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    Vec3 outward_normal = (rec.p - center(r.time())) / radius;
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;

    return true;
}

#endif //RAY_TRACING_MOVING_SPHERE_H
