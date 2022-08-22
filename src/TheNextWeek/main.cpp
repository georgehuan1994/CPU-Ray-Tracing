//
// Created by George on 2022/8/6.
//

#include "../math/vec3.h"
#include "../common/ray.h"
#include "../common/rtweekend.h"
#include "../common/camera.h"
#include "src/TheNextWeek/material.h"

#include "sphere.h"
#include "hittable_list.h"
#include "moving_sphere.h"
#include "bvh.h"

#include <iostream>


/// 发射射线，返回颜色
Color ray_color(const Ray &r, const hittable &world, int depth) {
    hit_record rec;

    // 迭代深度，也可以用 RR 作为终止条件
    if (depth <= 0) {
        return Color(0, 0, 0);
    }

    // 击中球体
    if (world.hit(r, 0.001, infinity, rec)) {
        // 直接输出法线颜色
        // return 0.5 * (rec.normal + Color(1, 1, 1));

        // 单位球体反射：中心点沿单位法线移动的单位球体
        // Point3 target = rec.p + rec.normal + random_unit_vector();

        // 单位半球反射
        // Point3 target = rec.p + random_in_hemisphere(rec.normal);

        // 输出半衰漫反射
        // return 0.5 * ray_color(Ray(rec.p, target - rec.p), world, depth - 1);

        Ray scattered;      // 散播射线
        Color attenuation;  // 能量衰减值 (材质反照率、漫射颜色)

        // 使用受击材质的属性为它们赋值，然后继续散播
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            // 递归，光线能量按材质内表面或外表面的衰减值衰减；表现为：随着弹射次数的增加，在最终颜色值中的叠加权重降低
            return attenuation * ray_color(scattered, world, depth - 1);
        }

        return Color(0, 0, 0);
    }

    // 未击中球体，即散播射线击中了天空，或第一次就击中了天空；方向越高 (y 分量越大)，颜色越蓝
    Vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0); // [0, 1]
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

/// 随机场景
hittable_list random_scene() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(Color(0.8, 0.8, 0.0));
    world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            Point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = Color::random() * Color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + Vec3(0, random_double(0, 0.5), 0);
                    world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = Color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(Color(0.4, 0.2, 0.1));
    world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

//    return hittable_list(make_shared<bvh_node>(world, 0.0, 1.0));
    return world;
}

int main() {
    // Image

    const auto aspect_ratio = 2.5;
    const int image_width = 860;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100; // 样本数，从每个像素发出的射线数 500
    const int max_depth = 50; // 50

    // World

    hittable_list world = random_scene();

    // Camera

    Point3 lookfrom(13, 2, 3);
    Point3 lookat(0, 0, 0);
    Vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    // Render

    FILE *f = fopen("image.ppm",
                    "w");  // 没有用 CMake 和 string，直接用的 MSBuild，改为文件 IO，添加 C/C++ 预处理器定义 _CRT_SECURE_NO_WARNINGS
    fprintf(f, "P3\n%d %d\n%d\n", image_width, image_height, 255);

    // 从左上角开始，从左到右逐行写入每个像素的颜色值
    for (int j = image_height; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;

        for (int i = 0; i < image_width; ++i) {
            Color pixel_color = Color(0, 0, 0);

            // 按样本数在每个像素中进行随机偏移采样
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (double(image_width) - 1);
                auto v = (j + random_double()) / (double(image_height) - 1);

                Ray ray = cam.get_ray(u, v);
                pixel_color += ray_color(ray, world, max_depth);
            }

            double r = pixel_color.x();
            double g = pixel_color.y();
            double b = pixel_color.z();

            double scale = 1.0 / samples_per_pixel;

            // Gamma 校正：gamma = 2.0，将颜色值提升为 (1/Gamma) 的幂
            r = sqrt(scale * r);
            g = sqrt(scale * g);
            b = sqrt(scale * b);

            int ir = 256 * clamp(r, 0.0, 0.999);
            int ig = 256 * clamp(g, 0.0, 0.999);
            int ib = 256 * clamp(b, 0.0, 0.999);

            fprintf(f, "%d %d %d ", ir, ig, ib);
        }
    }

    std::cerr << "\nDone.\n";
}
