#pragma once

#include "rtweekend.h"

class hit_record;

class material
{
public:
    virtual ~material() = default;

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    {
        return false;
    }
};

class lambertian : public material
{
public:
    lambertian(const color& a) : albedo(a) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) override
    {
        auto scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
};

class metal : public material
{
public:
    metal(const color& albedo) : albedo(albedo) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) override
    {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

private:
    color albedo;
};
