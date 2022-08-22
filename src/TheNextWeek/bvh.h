//
// Created by George on 2022/8/22.
//

#ifndef RAY_TRACING_BVH_H
#define RAY_TRACING_BVH_H

#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

class bvh_node : public hittable {
public:
    bvh_node();

    bvh_node(const hittable_list &list, double time0, double time1) :
            bvh_node(list.objects, 0, list.objects.size(), time0, time1) {}

    bvh_node(const std::vector<shared_ptr<hittable>> &src_objects,
             size_t start, size_t end, double time0, double time1);

    virtual bool hit(const Ray &r, double t_min, double t_max, hit_record &rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override;

public:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb box;
};

inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis) {
    aabb box_a;
    aabb box_b;

    if (!a->bounding_box(0, 0, box_a) || !b->bounding_box(0, 0, box_b))
        std::cerr << "No bounding box in bvh_node constructor.\n";

    return box_a.min().e[axis] < box_b.min().e[axis];
}

bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 2);
}

bvh_node::bvh_node(const std::vector<shared_ptr<hittable>> &src_objects,
                   size_t start, size_t end, double time0, double time1) {

    // 上个节点中的所有物体
    std::vector<shared_ptr<hittable>> objects = src_objects;

    // 在 x, y, z 中随机选一个轴
    int axis = random_int(0, 2);

    // comparison 方法
    auto comparator =
            (axis == 0) ? box_x_compare
                        : (axis == 1) ? box_y_compare
                                      : box_z_compare;
    // 节点中的物体数量
    size_t object_span = end - start;

    if (object_span == 1) {
        // 如果只有 1 个物体，在左右两个子节点中都包含这个物体
        left = right = objects[start];
    } else if (object_span == 2) {
        // 如果有 2 个物体，在左右两个子节点中各放一个物体
        if (comparator(objects[start], objects[start + 1])) {
            left = objects[start];
            right = objects[start + 1];
        } else {
            left = objects[start + 1];
            right = objects[start];
        }
    } else {
        // 如果有多个物体，按分割轴从小到大排序
        std::sort(objects.begin() + start, objects.begin() + end, comparator);

        // 对半分
        auto mid = start + object_span / 2;
        left = make_shared<bvh_node>(objects, start, mid, time0, time1);
        right = make_shared<bvh_node>(objects, mid, end, time0, time1);
    }

    aabb box_left, box_right;

    if (!left->bounding_box(time0, time1, box_left) || !right->bounding_box(time0, time1, box_right))
        std::cerr << "No bounding box in bvh_node constructor.\n";

    box = surrounding_box(box_left, box_right);
}

// 检查这个子节点是否被击中
bool bvh_node::hit(const Ray &r, double t_min, double t_max, hit_record &rec) const {
    if (!box.hit(r, t_min, t_max))
        return false;

    bool hit_left = left->hit(r, t_min, t_max, rec);
    bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
}

bool bvh_node::bounding_box(double time0, double time1, aabb &output_box) const {
    output_box = box;
    return true;
}

#endif //RAY_TRACING_BVH_H
