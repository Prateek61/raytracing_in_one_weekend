#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "threadpool.h"

thread_pool* thread_pool::instance_ = nullptr;
unsigned int thread_pool::num_threads = 0;

void render_image();

int main()
{
    render_image();
}

void render_image()
{
    auto& thread_pool = thread_pool::initialize();

    hittable_list world;

    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left = make_shared<metal>(color(0.8, 0.8, 0.8));
    auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2));

    world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(point3(0.0, 0.0, -1.2), 0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

    camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 640;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.render_threaded(world);
    //cam.render(world);
}