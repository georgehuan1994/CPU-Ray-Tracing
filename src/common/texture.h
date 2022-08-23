//
// Created by George on 2022/8/22.
//

#ifndef RAY_TRACING_TEXTURE_H
#define RAY_TRACING_TEXTURE_H

#include "rtweekend.h"
#include "perlin.h"

class texture {
public:
    virtual Color value(double u, double v, const Point3 &p) const = 0;
};

class solid_color : public texture {
public:
    solid_color() = default;

    solid_color(Color c) : color_value(c) {}

    solid_color(double red, double green, double blue) : solid_color(Color(red, green, blue)) {}

    virtual Color value(double u, double v, const Vec3 &p) const override {
        return color_value;
    }

private:
    Color color_value;
};

class checker_texture : public texture {
public:
    checker_texture() {}

    checker_texture(Color c1, Color c2) : even(make_shared<solid_color>(c1)),
                                          odd(make_shared<solid_color>(c2)) {}

    checker_texture(shared_ptr<texture> _even, shared_ptr<texture> _odd) : even(_even), odd(_odd) {}

    virtual Color value(double u, double v, const Point3 &p) const override {
        auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
        if (sines < 0)
            return odd->value(u, v, p);
        else
            return even->value(u, v, p);
    }

public:
    shared_ptr<texture> odd;
    shared_ptr<texture> even;

};

class noise_texture : public texture {
public:
    noise_texture() {}

    virtual Color value(double u, double v, const Point3 &p) const override {
        return Color(1, 1, 1) * noise.noise(p);
    }

private:
    perlin noise;
};

#endif //RAY_TRACING_TEXTURE_H
