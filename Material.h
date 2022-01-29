#pragma once

#include "utils.h"
//#include "Hittable.h"

struct HitRecord;

class Material {
public:
    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const = 0;
};


class Lambertian : public Material {
public:
    Lambertian(const Color& a) : albedo(a) {}

    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        Vec3 scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = Ray(rec.p, scatter_direction, r_in.time());
        attenuation = albedo;
        return true;
    }

public:
    Color albedo;
};


class Metal : public Material {
public:
    Metal(const Color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        Vec3 scatter_direction = reflect(unit_vector(r_in.direction()), rec.normal) + fuzz * random_in_unit_sphere();
        scattered = Ray(rec.p, scatter_direction, r_in.time());
        attenuation = albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }

public:
    Color albedo;
    double fuzz;
};


class Dielectric : public Material {
public:
    Dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        attenuation = Color(1.0, 1.0, 1.0);
        double n1_over_n2 = rec.front_face ? (1.0 / ir) : ir;


        Vec3 unit_I = unit_vector(r_in.direction());
        //Vec3 unit_N = unit_vector(rec.normal);
        double cos1 = fmin(dot(-unit_I, rec.normal), 1.0);
        double sin1 = sqrt(1 - cos1 * cos1);

        bool cannot_refract = n1_over_n2 * sin1 > 1.0;

        Vec3 refract_dir;
        if (cannot_refract || reflectance(cos1, n1_over_n2) > random_double()) {
            refract_dir = reflect(unit_I, rec.normal);
        }
        else {
            refract_dir = refract(unit_I, rec.normal, n1_over_n2);
        }

        scattered = Ray(rec.p, refract_dir, r_in.time());
        return true;
    }
public:
    double ir;  // Index of Refraction

private:
    static double reflectance(double cos, double ref_idx) {
        // Use Schlick's approximation for reflectance
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cos), 5);
    }
};
