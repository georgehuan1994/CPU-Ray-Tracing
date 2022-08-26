//
// Created by George on 2022/8/22.
//

#ifndef RAY_TRACING_TEXTURE_H
#define RAY_TRACING_TEXTURE_H

#include "rtweekend.h"
#include "rtw_stb_image.h"
#include "perlin.h"

#include <iostream>

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

    noise_texture(double sc) : scale(sc) {}

    virtual Color value(double u, double v, const Point3 &p) const override {
//        return Color(1, 1, 1) * 0.5 * (1 + noise.noise(scale * p));
//        return Color(1, 1, 1) * noise.turb(scale * p);
        return Color(1, 1, 1) * 0.5 * (1 + sin(scale * p.z() + 10 * noise.turb(scale * p)));
        return Color(1, 1, 1) * 0.5 * (1 + sin(scale * p.z() + 10 * noise.turb(p)));
    }

private:
    perlin noise;
    double scale;
};

class image_texture : public texture {
public:
    const static int bytes_per_pixel = 3;

    image_texture() : data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

    image_texture(const char *filename) {
        auto components_per_pixel = bytes_per_pixel;

        data = stbi_load(
                filename, &width, &height, &components_per_pixel, components_per_pixel);

        if (!data) {
            std::cerr << "ERROR Could not load texture image file '" << filename << "'.\n";
            width = height = 0;
        }

        bytes_per_scanline = bytes_per_pixel * width;
    }

    ~image_texture() {
        delete data;
    }

    virtual Color value(double u, double v, const Vec3 &p) const override {
        // 如果没有纹理数据，返回青色
        if (data == nullptr)
            return Color(0, 1, 1);

        // 将输入纹理坐标钳制为 [0,1] x [1,0]
        u = clamp(u, 0.0, 1.0);
        v = 1.0 - clamp(v, 0.0, 1.0);

        auto i = static_cast<int>(u * width);
        auto j = static_cast<int>(v * height);

        // 钳制整数映射，因为实际坐标应小于 1.0
        if (i >= width) i = width - 1;
        if (j >= height) j = height - 1;

        const auto color_scale = 1.0 / 255.0;
        auto pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;

        return Color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
    }

private:
    unsigned char *data;
    int width, height;
    int bytes_per_scanline;
};

#endif //RAY_TRACING_TEXTURE_H
