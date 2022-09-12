//
// Created by George on 2022/9/12.
//

#ifndef RAY_TRACING_PDF_H
#define RAY_TRACING_PDF_H

#include "vec3.h"
#include "onb.h"
#include "hittable.h"
#include "hittable_list.h"

class pdf {
public:
    virtual ~pdf() {}

    virtual double value(const Vec3 &direction) const = 0;

    virtual Vec3 generate() const = 0;
};

class cosine_pdf : public pdf {
public:
    cosine_pdf(const Vec3 &w) {
        uvw.build_from_w(w);
    }

    double value(const Vec3 &direction) const override {
        auto cosine = dot(unit_vector(direction), uvw.w());
        return (cosine <= 0) ? 0 : cosine / pi;
    }

    Vec3 generate() const override {
        return uvw.local(random_cosine_direction());
    }

public:
    onb uvw;
};

class hittable_pdf : public pdf {
public:
    hittable_pdf(shared_ptr<hittable> p, const Point3 &origin) : ptr(p), o(origin) {}
//    hittable_pdf(hittable _object, const Point3 &_origin) : objects(_object), origin(_origin) {}

    double value(const Vec3 &direction) const override {
        return ptr->pdf_value(o, direction);
    }

    Vec3 generate() const override {
        return ptr->random(o);
    }

public:
    Point3 o;
    shared_ptr<hittable> ptr;

//    const hittable &objects;
//    Point3 origin;
};

class mixture_pdf : public pdf {
public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
        p[0] = p0;
        p[1] = p1;
    }

    double value(const Vec3 &direction) const override {
        // p0 直接光，p1 全局光
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    Vec3 generate() const override {
        if (random_double() < 0.5) {
            return p[0]->generate();
        } else {
            return p[1]->generate();
        }
    }

public:
    shared_ptr<pdf> p[2];
};

class sphere_pdf : public pdf {
public:
    sphere_pdf() { }

    double value(const Vec3& direction) const override {
        return 1/ (4 * pi);
    }

    Vec3 generate() const override {
        return random_unit_vector();
    }
};

#endif //RAY_TRACING_PDF_H
