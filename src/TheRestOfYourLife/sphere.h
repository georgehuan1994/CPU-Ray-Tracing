#ifndef SPHERE
#define SPHERE

#include "hittable.h"
#include "../math/vec3.h"

class Sphere : public hittable {
public:
    Sphere() {}

    Sphere(Point3 cen, double r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};

    virtual bool hit(const Ray &r, double t_min, double t_max, hit_record &rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override;

    virtual double pdf_value(const Point3 &o, const Vec3 &v) const override;

    virtual Vec3 random(const Point3 &o) const override;

public:
    Point3 center;
    double radius;
    shared_ptr<material> mat_ptr;

private:
    static void get_sphere_uv(const Point3 &p, double &u, double &v) {
        // p: 单位球面上的一个点，以原点为中心
        // u: 返回从 X=-1 绕 Y 轴的角度值 [0,1]
        // v: 返回从 Y=-1 到 Y=+1 的角度值 [0,1]
        // <1 0 0> yields <0.50 0.50>   <-1  0  0> yields <0.00 0.50>
        // <0 1 0> yields <0.50 1.00>   < 0 -1  0> yields <0.50 0.00>
        // <0 0 1> yields <0.25 0.50>   < 0  0 -1> yields <0.75 0.50>

        auto theta = acos(-p.y());
        auto phi = atan2(-p.z(), p.x()) + pi;

        u = phi / (2 * pi);
        v = theta / pi;
    }
};

// Sphere 求交
bool Sphere::hit(const Ray &r, double t_min, double t_max, hit_record &rec) const {
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
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    Vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.mat_ptr = mat_ptr;

    return true;
}

// Sphere 包围盒
bool Sphere::bounding_box(double time0, double time1, aabb &output_box) const {
    output_box = aabb(
            center - Vec3(radius, radius, radius),
            center + Vec3(radius, radius, radius));
    return true;
}

double Sphere::pdf_value(const Point3 &o, const Vec3 &v) const {
    hit_record rec;
    if (!this->hit(Ray(o,v),0.001,infinity,rec)){
        return 0;
    }
    auto cos_theta_max = sqrt(1 - radius*radius/(center-o).length_squared());
    auto solid_angle = 2*pi*(1-cos_theta_max);

    return 1/solid_angle;
}

Vec3 Sphere::random(const Point3 &o) const {
    Vec3 diretion = center - o;
    auto distance_squared = diretion.length_squared();
    onb uvw;
    uvw.build_from_w(diretion);
    return uvw.local(random_to_sphere(radius, distance_squared));
}

#endif // !SPHERE

