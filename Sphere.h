#pragma once

#include "Hittable.h"
#include "vec3.h"


class Sphere : public Hittable {
public:
    Sphere() {}
    Sphere(const Point3 center_, const double radius_, const shared_ptr<Material> mat_ptr_) : 
        center(center_), radius(radius_), mat_ptr(mat_ptr_) {};

    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override;

public:
    Point3 center;
    double radius = 0;
    shared_ptr<Material> mat_ptr;
};

bool Sphere::hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const {
    Vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(r.direction(), oc);
    auto c = oc.length_squared() - radius * radius;
    auto discriminat = half_b * half_b - a * c;
    
    if (discriminat < 0) {
        return false;
    }
    auto sqrt_disc = sqrt(discriminat);
    auto root = (-half_b - sqrt_disc) / a;

    if (root < t_min || root > t_max) {
        root = (-half_b + sqrt_disc) / a;
        if (root < t_min || root > t_max) {
            return false;
        }
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    Vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;

    return true;
}
