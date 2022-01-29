#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "utils.h"
#include "HittableList.h"
#include "Sphere.h"
#include "Camera.h"
#include "Material.h"


using namespace std;


// Image Functions
void write_color(unsigned char* pixel_pos, Color pixel_color) {
    unsigned char& r = pixel_pos[0];
    unsigned char& g = pixel_pos[1];
    unsigned char& b = pixel_pos[2];


    // gamma-correct for gamma=2.0
    auto r_ = sqrt(pixel_color.x());
    auto g_ = sqrt(pixel_color.y());
    auto b_ = sqrt(pixel_color.z());

    r = static_cast<int>(256 * clamp(r_, 0.0, 0.999));
    g = static_cast<int>(256 * clamp(g_, 0.0, 0.999));
    b = static_cast<int>(256 * clamp(b_, 0.0, 0.999));
}


void write_image(const string output_path, unsigned char* image_data, const int& image_width, const int& image_height, const int& channel_count = 3) {

    stbi_write_png(output_path.c_str(), image_width, image_height, channel_count, image_data, 0);
    stbi_image_free(image_data);
}


HittableList random_scene() {
    HittableList objs;

    auto ground_material = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    objs.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            Point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<Material> sphere_mat;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = Color::random();
                    sphere_mat = make_shared<Lambertian>(albedo);
                    objs.add(make_shared<Sphere>(center, 0.2, sphere_mat));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = Color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_mat = make_shared<Metal>(albedo, fuzz);
                    objs.add(make_shared<Sphere>(center, 0.2, sphere_mat));
                }
                else {
                    // glass
                    sphere_mat = make_shared<Dielectric>(1.5);
                    objs.add(make_shared<Sphere>(center, 0.2, sphere_mat));
                }
            }
        }
    }

    auto mat1 = make_shared<Dielectric>(1.5);
    objs.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, mat1));

    auto mat2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    objs.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, mat2));
    
    auto mat3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    objs.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, mat3));

    return objs;
}


Color ray_color(const Ray& r, const HittableList& objects, int depth) {
    HitRecord rec;

    if (depth <= 0) {
        return Color(0, 0, 0);
    }

    if (objects.hit(r, 0.001, infinity, rec)) {
        //Point3 target = rec.p + rec.normal + random_unit_vector();
        Ray scattered;
        Color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * ray_color(scattered, objects, depth - 1);
        }
        else {
            return Color(0, 0, 0);
        }
    }
    else {
        Vec3 unit_direciton = unit_vector(r.direction());
        auto k = 0.5 * (unit_direciton.y() + 1.0);
        return (1.0 - k) * Color(1.0, 1.0, 1.0) + k * Color(0.5, 0.7, 1.0);
    }
}


int main() {
    // Image
    const double aspect_ratio = 16.0 / 9.0;
    //const double aspect_ratio = 3.0 / 2.0;
    const int image_width = 512;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 128;
    const int max_depth = 32;


    // World
    //HittableList world_objects = random_scene();

    //auto material_left = make_shared<Lambertian>(Color(0, 0, 1));
    //auto material_right = make_shared<Lambertian>(Color(1, 0, 1));
    //world_objects.add(make_shared<Sphere>(Point3(-R, 0, -1), R, material_left));
    //world_objects.add(make_shared<Sphere>(Point3(R, 0, -1), R, material_right));
    
    HittableList world_objects;
    auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto material_center = make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
    auto material_left = make_shared<Dielectric>(1.5);
    auto material_right = make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.0);
    world_objects.add(make_shared<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material_center));
    world_objects.add(make_shared<Sphere>(Point3(0.0, 0.0, 0.0), 0.2, material_center));
    world_objects.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.5), 0.2, material_center));
    //world_objects.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), -0.45, material_left));
    world_objects.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.5, material_left));
    world_objects.add(make_shared<Sphere>(Point3(1.0, 0.0, -1.0), 0.5, material_right));
    world_objects.add(make_shared<Sphere>(Point3(0.0, -100.5, -1.0), 100.0, material_ground));


    // Camera
    //Point3 lookfrom(13, 2, 3);
    //Point3 lookat(0, 0, 0);
    //Vec3 vup(0, 1, 0);
    //auto dist_to_focus = 10.0;
    //auto aperture = 0.1;
    //Camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    Camera cam(Point3(0,0,1), Point3(0,0,-1), Point3(0,1,0), 90, aspect_ratio, 0, 1);


    // Render  from left top to right bottom
    int channel_count = 3;
    auto* image_data = (unsigned char*)malloc(image_width * image_height * channel_count);
    for (int j = image_height-1; j >= 0 ; --j) {
        std::cerr << "\rScanlines remaining:" << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            unsigned char* pixel_offset = image_data + (i + image_width * (image_height - 1 - j)) * channel_count;
            Color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                double u = double(i + random_double()) / (image_width - 1);
                double v = double(j + random_double()) / (image_height - 1);
                Ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world_objects, max_depth);
            }
            pixel_color /= samples_per_pixel;
            write_color(pixel_offset, pixel_color);
        }
    }

    // write image
    string output_path = "./pic/output.png";
    write_image(output_path, image_data, image_width, image_height, channel_count);
}



//int main() {
//    // Image
//    const int w = 256;
//    const int h = 256;
//    const int channel_count = 3;
//
//    auto* image_data = (unsigned char*)malloc(w * h * channel_count);
//    // Render
//    for (int j = 0; j < h; ++j) {
//        for (int i = 0; i < w; ++i) {
//            unsigned char* pixel_offset = image_data + (i + w * j) * channel_count;
//            //unsigned char& r = pixel_offset[0];
//            //unsigned char& g = pixel_offset[1];
//            //unsigned char& b = pixel_offset[2];
//
//            Color c = color(double(i) / (w - 1), double(j) / (h - 1), 0.25);
//            write_color(pixel_offset, c);
//        }
//    }
//
//    // write image
//    string output_path = "./pic/output.png";
//    write_image(output_path, image_data, w, h, channel_count);
//}
