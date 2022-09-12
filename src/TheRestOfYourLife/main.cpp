//
// Created by George on 2022/8/6.
//

#include "../math/vec3.h"
#include "../common/ray.h"
#include "../common/rtweekend.h"
#include "../common/camera.h"
#include "src/TheRestOfYourLife/material.h"

#include "sphere.h"
#include "hittable_list.h"
#include "moving_sphere.h"
#include "bvh.h"
#include "texture.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "pdf.h"

#include <time.h>
#include <iostream>


/// 发射射线，返回颜色
Color ray_color(const Ray &r, const Color &background, const hittable &world, shared_ptr<hittable> lights, int depth) {
    hit_record rec;

    // 迭代深度，也可以用 RR 作为终止条件
    if (depth <= 0) {
        return Color(0, 0, 0);
    }

    if (!world.hit(r, 0.001, infinity, rec)) {
        return background;
    }

    // 击中球体
    // 直接输出法线颜色
    // return 0.5 * (rec.normal + Color(1, 1, 1));

    // 单位球体反射：中心点沿单位法线移动的单位球体
    // Point3 target = rec.p + rec.normal + random_unit_vector();

    // 单位半球反射
    // Point3 target = rec.p + random_in_hemisphere(rec.normal);

    // 输出半衰漫反射
    // return 0.5 * ray_color(Ray(rec.p, target - rec.p), world, depth - 1);

//    Ray scattered;      // 散播射线
//    Color albedo;       // 能量衰减值 (材质反照率、漫射颜色)
//    Color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);  // 自发光颜色
//    double pdf_val;

//    // 击中自发光材质
//    if (!rec.mat_ptr->scatter(r, rec, albedo, scattered, pdf_val)) {
//        return emitted;
//    }

//    // 光源上随机一点
//    auto on_light = Point3(random_double(214, 343), 554, random_double(227, 332));
//    // 着色点到光源方向
//    auto to_light = on_light - rec.p;
//    auto distance_squared = to_light.length_squared();
//    to_light = unit_vector(to_light);
//
//    if (dot(to_light, rec.normal) < 0) {
//        return emitted;
//    }
//
//    double light_area = (343 - 213) * (332 - 227);
//    auto light_cosine = fabs(to_light.y());
//    if (light_cosine < 0.000001) {
//        return emitted;
//    }
//
//    pdf = distance_squared / (light_cosine * light_area);
//    scattered = Ray(rec.p, to_light, r.time());

//    cosine_pdf p(rec.normal);
//    scattered = Ray(rec.p, p.generate(), r.time());
//    pdf_val = p.value(scattered.direction());

//    hittable_pdf light_pdf(lights, rec.p);
//    scattered = Ray(rec.p, light_pdf.generate(), r.time());
//    pdf_val = light_pdf.value(scattered.direction());

//    auto p0 = make_shared<hittable_pdf>(lights, rec.p);
//    auto p1 = make_shared<cosine_pdf>(rec.normal);
//    mixture_pdf mixed_pdf(p0, p1);
//    scattered = Ray(rec.p, mixed_pdf.generate(), r.time());
//    pdf_val = mixed_pdf.value(scattered.direction());

    scatter_record srec;
    Color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);  // 自发光颜色

    // 击中自发光材质
    if (!rec.mat_ptr->scatter(r, rec, srec)) {
        return emitted;
    }

    // 击中镜面材质
    if (srec.is_specular) {
        return srec.attenuation * ray_color(srec.specular_ray, background, world, lights, depth - 1);
    }

    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf p(light_ptr, srec.pdf_ptr);

    Ray scattered = Ray(rec.p, p.generate(), r.time()); // 散播射线
    auto pdf_val = p.value(scattered.direction());

    // 使用受击材质的属性为它们赋值，然后继续散播
    // 递归，光线能量按材质内表面或外表面的衰减值衰减；表现为：随着弹射次数的增加，在最终颜色值中的叠加权重降低
    return emitted + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered) * ray_color(scattered, background, world, lights, depth - 1) / pdf_val;
}

const char *file_name = "image.ppm";

/// 随机场景
hittable_list random_scene() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(Color(0.8, 0.8, 0.0));

//    // Plane
//    world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));
    auto checker = make_shared<checker_texture>(Color(0.1, 0.1, 0.1), Color(0.9, 0.9, 0.9));
    world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

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

    return hittable_list(make_shared<bvh_node>(world, 0.0, 1.0));
    return world;
}

hittable_list two_spheres() {
    hittable_list objects;

    auto checker = make_shared<checker_texture>(Color(0.1, 0.1, 0.1), Color(0.9, 0.9, 0.9));

    objects.add(make_shared<Sphere>(Point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<Sphere>(Point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list two_perlin_sphere() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<Sphere>(Point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list earth() {
    auto earth_texture = make_shared<image_texture>("Image/latlon-base-map.png");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<Sphere>(Point3(0, 0, 0), 2, earth_surface);

    return hittable_list(globe);
}

hittable_list simple_light() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<Sphere>(Point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(Color(4, 4, 4));
    objects.add(make_shared<Sphere>(Point3(0, 7, 0), 2, difflight));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

hittable_list cornell_box() {
    hittable_list objects;

    auto red = make_shared<lambertian>(Color(.65, .05, .05));
    auto white = make_shared<lambertian>(Color(.73, .73, .73));
    auto green = make_shared<lambertian>(Color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(Color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));

    // objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));

    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

//    shared_ptr<material> aluminum = make_shared<metal>(Color(0.8, 0.85, 0.88), 0.0);
//    shared_ptr<hittable> box1 = make_shared<box>(Point3(0, 0, 0), Point3(165, 330, 165), aluminum);

    shared_ptr<hittable> box1 = make_shared<box>(Point3(0, 0, 0), Point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vec3(265, 0, 295));
    objects.add(box1);

//    shared_ptr<hittable> box2 = make_shared<box>(Point3(0, 0, 0), Point3(165, 165, 165), white);
//    box2 = make_shared<rotate_y>(box2, -18);
//    box2 = make_shared<translate>(box2, Vec3(130, 0, 65));
//    objects.add(box2);

//    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<Sphere>(Point3(190, 90, 190), 90 , white));

    return objects;
}

hittable_list cornell_smoke() {
    hittable_list objects;

    auto red = make_shared<lambertian>(Color(.65, .05, .05));
    auto white = make_shared<lambertian>(Color(.73, .73, .73));
    auto green = make_shared<lambertian>(Color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(Color(7, 7, 7));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(Point3(0, 0, 0), Point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vec3(265, 0, 295));

    shared_ptr<hittable> box2 = make_shared<box>(Point3(0, 0, 0), Point3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, Vec3(130, 0, 65));

    objects.add(make_shared<constant_medium>(box1, 0.05, Color(0, 0, 0)));
    objects.add(make_shared<constant_medium>(box2, 0.05, Color(1, 1, 1)));

    return objects;
}

// 金属立方体 + 玻璃球
hittable_list cornell_box_cover1() {

    file_name = "cornell_box_cover1.ppm";
    hittable_list objects;

    auto red = make_shared<lambertian>(Color(.65, .05, .05));
    auto white = make_shared<lambertian>(Color(.73, .73, .73));
    auto green = make_shared<lambertian>(Color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(Color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    // 金属立方体
    shared_ptr<material> aluminum = make_shared<metal>(Color(0.8, 0.85, 0.88), 0.3);
    shared_ptr<hittable> box1 = make_shared<box>(Point3(0, 0, 0), Point3(165, 330, 165), aluminum);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vec3(265, 0, 295));
    objects.add(box1);

    // 玻璃球
    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<Sphere>(Point3(190, 90, 190), 90, glass));

    return objects;
}

// 白雾玻璃立方体 + 大理石球
hittable_list cornell_box_cover2() {

    file_name = "cornell_box_cover2.ppm";
    hittable_list objects;

    auto red = make_shared<lambertian>(Color(.65, .05, .05));
    auto white = make_shared<lambertian>(Color(.73, .73, .73));
    auto green = make_shared<lambertian>(Color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(Color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    // 白雾玻璃立方体
    shared_ptr<hittable> box1 = make_shared<box>(Point3(0, 0, 0), Point3(165, 330, 165), make_shared<dielectric>(1.5));
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vec3(265, 0, 295));
    objects.add(box1);
    objects.add(make_shared<constant_medium>(box1, 0.1, Color(.73, .73, .73)));

    // 大理石球
    auto pertext = make_shared<noise_texture>(0.05);
    objects.add(make_shared<Sphere>(Point3(190, 90, 190), 90, make_shared<lambertian>(pertext)));

    return objects;
}

// 聚集立方体 + 雾玻璃球
hittable_list cornell_box_cover3() {

    file_name = "cornell_box_cover3.ppm";
    hittable_list objects;

    auto red = make_shared<lambertian>(Color(.65, .05, .05));
    auto white = make_shared<lambertian>(Color(.73, .73, .73));
    auto green = make_shared<lambertian>(Color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(Color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    // 聚集方块
    hittable_list boxes;
    int ns = 4000;
    for (int j = 0; j < ns; j++) {
        boxes.add(make_shared<Sphere>(Point3(
                random_double(0.0, 165.0),
                random_double(0.0, 330.0),
                random_double(0.0, 165.0)), 10, white));
    }
    objects.add(make_shared<translate>(
                        make_shared<rotate_y>(
                                make_shared<bvh_node>(boxes, 0.0, 1.0), 15),
                        Vec3(265, 0, 295)
                )
    );

    // 雾玻璃球
    auto boundary = make_shared<Sphere>(Point3(190, 90, 190), 90, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.2, Color(0.2, 0.4, 0.9)));

    return objects;
}

hittable_list final_scene() {

    hittable_list objects;

    // 地面
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(Color(0.48, 0.83, 0.53));
    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; ++i) {
        for (int j = 0; j < boxes_per_side; ++j) {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<box>(Point3(x0, y0, z0), Point3(x1, y1, z1), ground));
        }
    }
    objects.add(make_shared<bvh_node>(boxes1, 0, 1));

    // 顶灯
    auto light = make_shared<diffuse_light>(Color(7, 7, 7));
    objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    // 运动球
    auto center1 = Point3(400, 400, 200);
    auto center2 = center1 + Vec3(30, 0, 0);
    auto moving_sphere_material = make_shared<lambertian>(Color(0.7, 0.3, 0.1));
    objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    // 玻璃球
    objects.add(make_shared<Sphere>(Point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));

    // 金属球(右)
    objects.add(make_shared<Sphere>(Point3(0, 150, 145), 50, make_shared<metal>(Color(0.8, 0.8, 0.9), 1.0)));

    // 蓝色玻璃球
    auto boundary = make_shared<Sphere>(Point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.2, Color(0.2, 0.4, 0.9)));

    // 全局雾
    boundary = make_shared<Sphere>(Point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    objects.add(make_shared<constant_medium>(boundary, .0001, Color(1, 1, 1)));

    // 贴图纹理球
    auto emat = make_shared<lambertian>(make_shared<image_texture>("Image/latlon-base-map.png"));
    objects.add(make_shared<Sphere>(Point3(400, 200, 400), 100, emat));

    // 噪声纹理球
    auto pertext = make_shared<noise_texture>(0.1);
    objects.add(make_shared<Sphere>(Point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

    // 聚集方块
    hittable_list boxes2;
    auto white = make_shared<lambertian>(Color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<Sphere>(Point3::random(0, 165), 10, white));
    }
    objects.add(make_shared<translate>(
                        make_shared<rotate_y>(
                                make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
                        Vec3(-100, 270, 395)
                )
    );

    return objects;
}

hittable_list final_scene2() {

    hittable_list world;

    auto ground_material = make_shared<lambertian>(Color(0.8, 0.8, 0.0));

    // Plane
    world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

    // 小球
    hittable_list boxes1;
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
                    if (choose_mat < 0.75) {
                        boxes1.add(make_shared<Sphere>(center, 0.2, sphere_material));
                    } else {
                        boxes1.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                    }
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = Color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    boxes1.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    boxes1.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }
    world.add(make_shared<bvh_node>(boxes1, 0, 1));


    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

//    auto material2 = make_shared<lambertian>(make_shared<image_texture>("Image/marsmap.jpg"));
//    world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto pertext = make_shared<noise_texture>(0.1);
    world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, make_shared<lambertian>(pertext)));

    auto material3 = make_shared<metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    // 灯
    auto light = make_shared<diffuse_light>(Color(7, 7, 7));
    world.add(make_shared<Sphere>(Point3(0, 4, 0), 1.0, light));

//    auto light = make_shared<diffuse_light>(Color(7, 7, 7));
//    world.add(make_shared<xz_rect>(-5, 5, -0.5, 0.5, 4, light));

    // 全局雾
    auto boundary = make_shared<Sphere>(Point3(0, 0, 0), 50, make_shared<dielectric>(1.5));
    world.add(make_shared<constant_medium>(boundary, .0001, Color(1, 1, 1)));

    return world;
}

hittable_list cornell_box_new() {
    hittable_list objects;

    auto red = make_shared<lambertian>(Color(.65, .05, .05));
    auto blue = make_shared<lambertian>(Color(.23, .23, .8));
    auto white = make_shared<lambertian>(Color(1, 1, 1));
    auto green = make_shared<lambertian>(Color(0.0, .63, 0.0));

    auto light_white = make_shared<diffuse_light>(Color(15, 15, 16));
    auto light_blue = make_shared<diffuse_light>(Color(0.5, 9, 9));
    auto light_yellow = make_shared<diffuse_light>(Color(9, 9, 0.5));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, blue));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, green));

    // objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light_white));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light_white)));
    objects.add(make_shared<flip_face>(make_shared<yz_rect>(35, 40, 0, 555, 554, light_blue)));
    objects.add(make_shared<yz_rect>(110, 115, 0, 555, 1, light_yellow));

    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<material> aluminum = make_shared<metal>(Color(0.8, 0.85, 0.88), 0.3);
    shared_ptr<hittable> box1 = make_shared<box>(Point3(0, 0, 0), Point3(165, 330, 165), aluminum);

//    shared_ptr<hittable> box1 = make_shared<box>(Point3(0, 0, 0), Point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, Vec3(265, 0, 295));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(Point3(0, 0, 0), Point3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, Vec3(130, 0, 65));
    objects.add(box2);

//    auto glass = make_shared<dielectric>(1.5);
//    objects.add(make_shared<Sphere>(Point3(190, 165+80, 190), 80 , red));

    auto boundary = make_shared<Sphere>(Point3(190, 165+80, 190), 80, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.2, Color(1, 0, 0)));

    return objects;
}

int main() {

    clock_t start, end;
    start = clock();

    // Image

    auto aspect_ratio = 2.5;
    int image_width = 860;
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int samples_per_pixel = 500; // 样本数，从每个像素发出的射线数 500
    const int max_depth = 50; // 50

    // World & Camera

    hittable_list world;

    shared_ptr<hittable> lights = make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>());
//    shared_ptr<hittable> lights = make_shared<Sphere>(Point3(190, 90, 190), 90, shared_ptr<material>());

//    auto lights = make_shared<hittable_list>();
//    lights->add(make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>()));
//    lights->add(make_shared<Sphere>(Point3(190, 90, 190), 90, shared_ptr<material>()));

//    lights->add(make_shared<Sphere>(Point3(190, 165+80, 190), 80, shared_ptr<material>()));
//    lights->add(make_shared<xz_rect>(35, 40, 0, 555, 554, shared_ptr<material>()));
//    lights->add(make_shared<xz_rect>(110, 115, 0, 555, 1, shared_ptr<material>()));


    Point3 lookfrom;
    Point3 lookat;
    auto vfov = 40.0;
    auto dist_to_focus = 10.0;
    auto aperture = 0.0;
    Vec3 vup(0, 1, 0);
    Color background(0, 0, 0);

    switch (0) {
        case 1:
            world = random_scene();
            background = Color(0.70, 0.80, 1.00);
            lookfrom = Point3(13, 2, 3);
            lookat = Point3(0, 0, 0);
            vfov = 20.0;
            aperture = 0.1;
            break;

        case 2:
            world = two_spheres();
            background = Color(0.70, 0.80, 1.00);
            lookfrom = Point3(13, 2, 3);
            lookat = Point3(0, 0, 0);
            vfov = 20.0;
            break;

        case 3:
            world = two_perlin_sphere();
            background = Color(0.70, 0.80, 1.00);
            lookfrom = Point3(13, 2, 3);
            lookat = Point3(0, 0, 0);
            vfov = 20.0;
            break;

        case 4:
            world = earth();
            background = Color(0.70, 0.80, 1.00);
            lookfrom = Point3(13, 2, 3);
            lookat = Point3(0, 0, 0);
            vfov = 20.0;
            break;

        case 5:
            world = simple_light();
            background = Color(0.0, 0.0, 0.0);
            lookfrom = Point3(26, 3, 6);
            lookat = Point3(0, 2, 0);
            vfov = 20.0;
            break;

        default:
        case 6:
            world = cornell_box();
//            world = cornell_box_new();
//            world = cornell_box_cover1();
//            world = cornell_box_cover2();
//            world = cornell_box_cover3();

            aspect_ratio = 1.0;
            image_width = 512;
            image_height = 512;
            samples_per_pixel = 200; // 8192
            background = Color(0, 0, 0);
            lookfrom = Point3(278, 278, -800);
            lookat = Point3(278, 278, 0);
            vfov = 40.0;
            break;

        case 7:
            world = cornell_smoke();
            aspect_ratio = 1.0;
            image_width = 512;
            image_height = 512;
            samples_per_pixel = 1024;
            background = Color(0, 0, 0);
            lookfrom = Point3(278, 278, -800);
            lookat = Point3(278, 278, 0);
            vfov = 40.0;
            break;


        case 8:
            world = final_scene2();
            samples_per_pixel = 10000;
            lookfrom = Point3(13, 2, 3);
            lookat = Point3(0, 0, 0);
            vfov = 20.0;
            aperture = 0.1;

            break;
    }

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    // Render

    // 没有用 CMake 和 string，直接用的 MSBuild，改为文件 IO，添加 C/C++ 预处理器定义 _CRT_SECURE_NO_WARNINGS
    FILE *f = fopen(file_name, "w");
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
                pixel_color += ray_color(ray, background, world, lights, max_depth);
            }

            double r = pixel_color.x();
            double g = pixel_color.y();
            double b = pixel_color.z();

//            if (r != r) r = 0.0;
//            if (g != g) g = 0.0;
//            if (b != b) b = 0.0;

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

    end = clock();   //结束时间
    std::cerr << "\ntime = " << double(end - start) / CLOCKS_PER_SEC << "s\n";
}
