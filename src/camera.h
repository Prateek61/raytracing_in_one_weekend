#pragma once

#include "rtweekend.h"

#include "hittable.h"
#include "material.h"
#include "threadpool.h"

class camera
{
public:
    double aspect_ratio = 1.0; // Aspect ratio of the rendered image
    int image_width = 100; // Rendered image width
    int samples_per_pixel = 10; // Number of samples per pixel
    int max_depth = 10; // Maximum number of bounces for a ray

    void render(const hittable& world);
    void render_threaded(const hittable& world);
private:
    int image_height;   // Rendered image height
    double pixel_sample_scale; // Color scale factor for a sum of pixel samples
    point3 center;      // Camera center
    point3 pixel00_loc; // Location of the upper left pixel
    vec3 pixel_delta_u; // Horizontal delta from pixel to pixel
    vec3 pixel_delta_v; // Vertical delta from pixel to pixel

    void initialize();
    color ray_color(const ray& r, int depth, const hittable& world) const;
    ray get_ray(int i, int j) const;
    vec3 sample_square() const;
};

inline void camera::render_threaded(const hittable& world)
{
    initialize();
    auto& thread_pool = thread_pool::get_instance();
    unsigned int num_threads = thread_pool.num_threads;
    int batches_per_thread = image_height / num_threads;

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    std::vector <std::vector<color>> colors(image_width, std::vector<color>(image_height));

    for (int t = 0; t < num_threads; t++)
    {
        thread_pool.queue_job([this, &world, &colors, t, num_threads, batches_per_thread](){
            int start = t * batches_per_thread;
            int end = (t + 1) * batches_per_thread;
            if (t == num_threads - 1)
            {
                end = image_height;
            }

            for (int j = start; j < end; j++)
            {
                std::clog << "\rScanlines remaining: " << (end - j) << ' ' << std::flush;
                for (int i = 0; i < image_width; i++)
                {
                    color pixel_color(0, 0, 0);
                    for (int sample = 0; sample < samples_per_pixel; sample++)
                    {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }

                    colors[i][j] = pixel_color;
                }
            }
        });
    }

    while (thread_pool.busy())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    thread_pool.stop();

    for (int j = 0; j < image_height; j++)
    {
        for (int i = 0; i < image_width; i++)
        {
            write_color(std::cout, colors[i][j] * pixel_sample_scale);
        }
    }
    std::clog << "\rDone.                 \n";
}

inline void camera::render(const hittable &world)
{
    initialize();

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    std::vector<std::vector<color>> colors(image_width, std::vector<color>(image_height));

    for (int j = 0; j < image_height; j++) {
        
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            color pixel_color(0, 0, 0);
            for (int sample = 0; sample < samples_per_pixel; sample++)
            {
                ray r = get_ray(i, j);
                pixel_color += ray_color(r, max_depth, world);
            }

            colors[i][j] = pixel_color;
            write_color(std::cout, pixel_color * pixel_sample_scale);
        }
    }

    std::clog << "\rDone.                 \n";
}

inline void camera::initialize()
{
    image_height = int(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    pixel_sample_scale = 1.0 / samples_per_pixel;

    center = point3(0, 0, 0);

    // Determine viewport dimensions
    auto focal_length = 1.0;
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (double(image_width) / image_height);

    // Calculate the vectors across the horizontal and down the vertical dimensions of the viewport
    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0);

    // Calculate the horizontal and vertical deltas from pixel to pixel
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel
    auto viewport_upper_left = center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
}

color camera::ray_color(const ray &r, int depth, const hittable &world) const
{
    if (depth <= 0)
        return color(0, 0, 0);

    hit_record rec;

    if (world.hit(r, interval(0.001, infinity), rec))
    {
        ray scattered;
        color attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, depth - 1, world);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

inline ray camera::get_ray(int i, int j) const
{
    // Construct a camera ray originating from the origin and directed at randomly sampled
    // points around the pixel location i, j.

    auto offset = sample_square();
    auto pixel_sample = pixel00_loc + (i + offset.x()) * pixel_delta_u + (j + offset.y()) * pixel_delta_v;

    auto ray_origin = center;
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
}

inline vec3 camera::sample_square() const
{
    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    return vec3(random_double() - 0.5, random_double() - 0.5, 0);
}
