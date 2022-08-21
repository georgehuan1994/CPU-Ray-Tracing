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
        src/TheNextWeek/main.cpp src/TheNextWeek/moving_sphere.h)