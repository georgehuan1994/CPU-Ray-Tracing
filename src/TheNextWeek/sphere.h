#ifndef SPHERE
#define SPHERE

#include "hittable.h"
#include "../math/vec3.h"

class Sphere : public hittable
{
public:
	Sphere() {}
    Sphere(Point3 cen, double r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};

	virtual bool hit(const Ray& r, double t_min, double t_max, hit_record& rec) const override;

public:
	Point3 center;
	double radius;
    shared_ptr<material> mat_ptr;
};

bool Sphere::hit(const Ray& r, double t_min, double t_max, hit_record& rec) const
{
    Vec3 oc = r.origin() - center;              // 射线起点到球体中心

    // 求根公式
    // auto a = dot(r.direction(), r.direction());
    // auto b = 2.0 * dot(oc, r.direction());
    // auto c = dot(oc, oc) - radius * radius;
    // auto discriminant = b * b - 4 * a * c;

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
    Vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;

    return true;
}


#endif // !SPHERE

