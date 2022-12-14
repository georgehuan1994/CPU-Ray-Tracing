cmake_minimum_required(VERSION 3.23)
project(ray_tracing)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(src)
include_directories(src/common)
include_directories(src/InOneWeekend)
include_directories(src/TheNextWeek)
include_directories(src/math)
include_directories(thirdparty)

add_executable(InOneWeek
        src/common/camera.h
        src/common/color.h
        src/common/ray.h
        src/common/rtweekend.h
        src/InOneWeekend/material.h
        src/InOneWeekend/hittable.h
        src/InOneWeekend/hittable_list.h
        src/InOneWeekend/main.cpp
        src/InOneWeekend/sphere.h
        src/math/vec3.h)

add_executable(TheNextWeek
        src/common/camera.h
        src/common/color.h
        src/common/ray.h
        src/common/rtweekend.h
        src/TheNextWeek/material.h
        src/TheNextWeek/hittable.h
        src/TheNextWeek/hittable_list.h
        src/TheNextWeek/sphere.h
        src/math/vec3.h
        src/TheNextWeek/main.cpp
        src/TheNextWeek/moving_sphere.h
        src/common/aabb.h
        src/TheNextWeek/bvh.h
        src/common/texture.h
        src/common/perlin.h
        src/common/external/stb_image.h
        src/common/external/stb_image_write.h
        src/common/rtw_stb_image.h
        src/TheNextWeek/aarect.h
        src/TheNextWeek/box.h
        src/TheNextWeek/constant_medium.h)

add_executable(TheRestOfYourLife
        src/common/camera.h
        src/common/color.h
        src/common/ray.h
        src/common/rtweekend.h
        src/TheRestOfYourLife/material.h
        src/TheRestOfYourLife/hittable.h
        src/TheRestOfYourLife/hittable_list.h
        src/TheRestOfYourLife/sphere.h
        src/math/vec3.h
        src/TheRestOfYourLife/main.cpp
        src/TheRestOfYourLife/moving_sphere.h
        src/common/aabb.h
        src/TheRestOfYourLife/bvh.h
        src/common/texture.h
        src/common/perlin.h
        src/common/external/stb_image.h
        src/common/external/stb_image_write.h
        src/common/rtw_stb_image.h
        src/TheRestOfYourLife/aarect.h
        src/TheRestOfYourLife/box.h
        src/TheRestOfYourLife/constant_medium.h
        src/TheRestOfYourLife/onb.h src/TheRestOfYourLife/pdf.h)