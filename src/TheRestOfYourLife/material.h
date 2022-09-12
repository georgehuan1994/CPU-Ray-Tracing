#ifndef MATERIAL_H
#define MATERIAL_H

#include "../common/rtweekend.h"
#include "hittable.h"
#include "texture.h"
#include "onb.h"
#include "pdf.h"

struct hit_record;

struct scatter_record {
    Ray specular_ray;
    bool is_specular;
    Color attenuation;
    shared_ptr<pdf> pdf_ptr;
};

class material {
public:
    virtual Color emitted(const Ray &r_in, const hit_record &rec, double u, double v, const Point3 &p) const {
        return Color(0, 0, 0);
    }

    virtual bool scatter(const Ray &r_in, const hit_record &rec, scatter_record &srec) const {
        return false;
    };

    virtual double scattering_pdf(const Ray &r_in, const hit_record &rec, const Ray &scattered) const {
        return 0;
    }
};

class lambertian : public material {
public:
    lambertian(const Color &a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(const Ray &r_in, const hit_record &rec, scatter_record &srec) const override {

        // 散播方向 (未归一化)，单位球体反射：与受击点相切的单位球体
        // auto scatter_direction = rec.normal + random_unit_vector();
        // auto scatter_direction = random_in_hemisphere(rec.normal);

        // 散播方向 (未归一化)，单位半球反射
        // auto scatter_direction = rec.p + random_in_hemisphere(rec.normal);

        // 如果散播方向 (未归一化) 三个分量接近 0，将法线方向作为散播方向
        // if (scatter_direction.near_zero()) {
        //     scatter_direction = rec.normal;
        // }

//        onb uvw;
//        uvw.build_from_w(rec.normal);
//        auto direction = uvw.local(random_cosine_direction());
//        scattered = Ray(rec.p, unit_vector(direction), r_in.time());
//        // scattered = Ray(rec.p, scatter_direction, r_in.time());
//        alb = albedo->value(rec.u, rec.v, rec.p);

        // 采样 pdf
        // pdf = dot(uvw.w(), scattered.direction()) / pi;  // cos(theta)^2 / PI
        // pdf = 0.5 / pi;  // 1 / 2PI

        srec.is_specular = false;
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        return true;
    }

    // 散射 pdf
    double scattering_pdf(const Ray &r_in, const hit_record &rec, const Ray &scattered) const {
        auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
        return cosine < 0 ? 0 : cosine / pi;
        // return cosine < 0 ? 0 : 0.5 / pi;
    }

public:
    shared_ptr<texture> albedo;
};

class metal : public material {
public:
    metal(const Color &a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const Ray &r_in, const hit_record &rec, scatter_record &srec) const override {
        Vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        // scattered = Ray(rec.p, reflected);
        // scattered = Ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
        // attenuation = albedo;
        // return (dot(scattered.direction(), rec.normal) > 0);
        srec.specular_ray = Ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        srec.attenuation = albedo;
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        return true;
    }

public:
    Color albedo;
    double fuzz;
};

class dielectric : public material {
public:
    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(const Ray &r_in, const hit_record &rec, scatter_record &srec) const override {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.attenuation = Color(1.0, 1.0, 1.0);
        double refraciton_ratio = rec.front_face ? (1.0 / ir) : ir;

        Vec3 unit_direction = unit_vector(r_in.direction());

        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraciton_ratio * sin_theta > 1.0;
        Vec3 direction;
        if (cannot_refract || reflectance(cos_theta, refraciton_ratio) > random_double()) {
            direction = reflect(unit_direction, rec.normal);
        } else {
            direction = refract(unit_direction, rec.normal, refraciton_ratio);
        }

        srec.specular_ray = Ray(rec.p, direction, r_in.time());
        return true;
    }

public:
    double ir; // Index of Refreaction

private:
    static double reflectance(double cosine, double ref_idx) {
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

class diffuse_light : public material {
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}

    diffuse_light(Color c) : emit(make_shared<solid_color>(c)) {}

    Color emitted(const Ray &r_in, const hit_record &rec, double u, double v, const Point3 &p) const override {
        if (!rec.front_face) {
            return Color(0, 0, 0);
        }
        return emit->value(u, v, p);
    }

public:
    shared_ptr<texture> emit;
};

class isotropic : public material {
public:
    isotropic(Color c) : albedo(make_shared<solid_color>(c)) {}

    isotropic(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(const Ray &r_in, const hit_record &rec, scatter_record& srec) const override {
//        scattered = Ray(rec.p, random_in_unit_sphere(), r_in.time());
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = make_shared<sphere_pdf>();
        srec.is_specular = false;
        return true;
    }

    double scattering_pdf(const Ray& r_in, const hit_record& rec, const Ray& scattered) const override {
        return 1 / (4 * pi);
    }

public:
    shared_ptr<texture> albedo;
};

#endif